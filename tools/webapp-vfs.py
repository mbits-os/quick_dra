# Copyright (c) 2026 Marcin Zdun
# This code is licensed under MIT license (see LICENSE for details)

import os
import re
import shutil
import string
import subprocess
import sys
from contextlib import contextmanager
from datetime import datetime
from pathlib import Path

source_root = Path(__file__).parent.parent.resolve()
web_dist = source_root / "data" / "web" / "dist"
tarball = source_root / "libs" / "libgui" / "src" / "tarball.cpp"
whole_line_len = 120
tarball_prefix = "		    "
line_len = whole_line_len - 15
printable = string.ascii_letters + string.digits + string.punctuation + " "

__yarn_exe = shutil.which("yarn") or "yarn"


@contextmanager
def __cd(path: str | Path):
    curr = os.getcwd()
    os.chdir(path)
    try:
        yield None
    finally:
        os.chdir(curr)


def __yarn(*cmd: str):
    proc = subprocess.run([__yarn_exe, *cmd], shell=False, check=False)
    if proc.returncode:
        raise SystemExit(1)


def __file_slug(path: Path):
    url = path.relative_to(web_dist).as_posix()
    return re.sub(r"[^a-zA-Z0-9_]", "_", url)


__simple_escape_seq_char = {
    ord("\\"): "\\\\",
    ord('"'): '\\"',
    ord("\n"): "\\n",
    ord("\r"): "\\r",
    ord("\t"): "\\t",
    ord("\v"): "\\v",
}


def __tarball_chr(ch: int, printed_hex: bool):
    if ch in __simple_escape_seq_char:
        return (__simple_escape_seq_char[ch], False, False)

    if ch < 127 and chr(ch) in printable:
        return (chr(ch), printed_hex, False)

    return (f"\\x{ch:02X}", False, True)


def __lines(byte_contents: bytes, max_length: int):
    output = ""
    printed_hex = False
    for ch in byte_contents:
        repl, break_str, printed_hex = __tarball_chr(ch, printed_hex)
        if break_str and output:
            yield output
            output = ""
        next_output = f"{output}{repl}"
        if len(next_output) > max_length:
            yield output
            output = repl
        else:
            output = next_output

    if output:
        yield output


def __tarball_file(byte_contents: bytes):
    for line in __lines(byte_contents, line_len):
        yield f'{tarball_prefix}"{line}"'


def __tarball_text():
    dist_files: list[Path] = []

    for root, _, filenames in web_dist.walk():
        dist_files.extend(root / name for name in filenames)

    dist_files = list(sorted(dist_files))

    output = """#include <array>
#include <quick_dra/gui/vfs.hpp>

using namespace std::literals;

namespace quick_dra::gui {
	namespace contents {
"""

    index = 0
    for filename in dist_files:
        index += 1

        if index > 1:
            output += "\n"

        byte_contents = filename.read_bytes()

        output += f"""		static constexpr auto file_{__file_slug(filename)} =
{'\n'.join(__tarball_file(byte_contents))}
		    ""sv;
"""

    output += """	}  // namespace contents

	static constexpr entry files[] = {
"""

    for filename in dist_files:
        url = filename.relative_to(web_dist).as_posix().replace('"', '\\"')
        output += f'	    {{.name = "/{url}"sv, .contents = contents::file_{__file_slug(filename)}}},\n'

    output += """	};

	void virtual_filesystem::install_global_data() { set_global(build(files)); }
}  // namespace quick_dra::gui
"""
    return output


def __get_current_tarball():
    try:
        text = tarball.read_text()
        pos = text.find("\n#include ")
        pos = 0 if pos < 0 else pos + 1
        return text[pos:]
    except FileNotFoundError:
        return None


def __write_new_tarball(text: str):
    this_year = datetime.now().year
    text = f"""// Copyright (c) {this_year} Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)
// This file is auto-generated. To recreate, call
//
//     python tools/webapp-vfs.py
//

{text}"""

    tarball.write_bytes(text.encode("utf-8"))
    print("-- updated", tarball)


def __wants_help():
    return "-h" in sys.argv or "--help" in sys.argv


def __is_forced():
    return "--force" in sys.argv


def __install_needed():
    return "--install" in sys.argv


def __build_needed():
    return "--no-build" not in sys.argv


def rebuild(with_install: bool):
    with __cd(web_dist.parent):
        if with_install:
            __yarn("install")
        __yarn("build:dist")


def write_tarball(force: bool):
    text = __tarball_text()
    recreate = force
    if not recreate:
        current = __get_current_tarball()
        recreate = current != text
    if recreate:
        __write_new_tarball(text)


def __main():
    if __wants_help():
        print(
            f"python {sys.argv[0]} [-h] [--force] [--install] [--no-build]",
            file=sys.stderr,
        )
        sys.exit(0)
    if __build_needed():
        rebuild(__install_needed())
    write_tarball(__is_forced())


if __name__ == "__main__":
    __main()
