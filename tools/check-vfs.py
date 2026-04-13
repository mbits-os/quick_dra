# Copyright (c) 2026 Marcin Zdun
# This code is licensed under MIT license (see LICENSE for details)

import subprocess
import sys

files_to_check = [
    "data/web/yarn.lock",
    "libs/libgui/src/tarball.cpp",
]

proc = subprocess.run(
    ["git", "diff", "--diff-filter=d", "--name-only", "-z"],
    shell=False,
    check=False,
    capture_output=True,
    encoding="utf-8",
)

if proc.returncode != 0:
    if proc.stdout:
        print(proc.stdout.rstrip(), file=sys.stdout)
    if proc.stderr:
        print(proc.stderr.rstrip(), file=sys.stderr)
    raise SystemExit(1)

filelist = {file for file in proc.stdout.split("\0") if file}

checked = [check for check in files_to_check if check in filelist]
if checked:
    print(
        f"There {'are files' if len(checked) != 1 else 'is a file'}, which need{'' if len(checked) != 1 else 's'} to be checked-in before push:"
    )
    for check in checked:
        print(f" - {check}")
    raise SystemExit(1)
