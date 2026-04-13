import json
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable, cast

import yaml

cwd = Path().resolve()
src_roots = [cwd / "libs"]

if len(sys.argv) < 3:
    print(
        f"Usage: {Path(sys.argv[0]).name} <clang-tidy-exec> <build-dir> [clang-tidy-args...]",
        file=sys.stderr,
    )
    raise SystemExit(1)

clang_tidy_exec = sys.argv[1]
build_dir = sys.argv[2]


@dataclass
class TidyRun:
    config_file: str
    scope: list[str] = field(default_factory=list)


def _get_files():
    file_list: list[Path] | None = None

    try:
        file_list_text = (cwd / ".file-list").read_text().rstrip()
        if file_list_text:
            file_list = [
                Path(line.strip()).resolve() for line in file_list_text.split("\n")
            ]
    except FileNotFoundError:
        # It's OK for this file not to exist
        pass

    path = Path(build_dir, "compile_commands.json")
    try:
        commands = json.loads(path.read_bytes())
        mapping = map(
            lambda command: Path(command.get("file", "")).resolve(),
            cast(list[dict[str, str]], commands),
        )
        if file_list is None:
            return mapping
        return filter(lambda name: name in file_list, mapping)
    except FileNotFoundError:
        preset = build_dir.split("/")[-1]
        print(
            f"""{path} not found, please run

   cmake --preset {preset} -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
""",
            file=sys.stderr,
        )
        sys.exit(1)


def _get_run_dir(dirname: Path):
    while not (dirname / ".clang-tidy").exists():
        parent = dirname.parent
        if parent == dirname:
            return None
        dirname = parent
    return dirname


def _get_runs(files: Iterable[Path]):
    runs: dict[Path, TidyRun] = {}
    file_dir_to_root_dir: dict[Path, Path] = {}
    for filename in files:
        dirname = filename.parent
        if dirname not in file_dir_to_root_dir:
            dst = _get_run_dir(dirname)
            if dst is None:
                continue
            file_dir_to_root_dir[dirname] = dst
        scope_dir = file_dir_to_root_dir[dirname]
        if scope_dir not in runs:
            runs[scope_dir] = TidyRun(
                config_file=(scope_dir / ".clang-tidy").as_posix()
            )
        runs[scope_dir].scope.append(str(filename))
    return runs


runs = _get_runs(_get_files())
if not runs:
    print("Nothing to check now")

counter = 0
for run in runs.values():
    counter += 1
    checks = (
        cast(
            dict[str, str], yaml.safe_load(Path(run.config_file).read_bytes()) or {}
        ).get("Checks")
        or "-*"
    )

    if checks == "-*":
        print(
            f"[{counter}/{len(runs)}] skip",
            str(Path(run.config_file).relative_to(cwd)),
        )
        continue

    base_cmd = [
        clang_tidy_exec,
        "-p",
        build_dir,
        "--use-color",
        "--config-file",
        str(Path(run.config_file).relative_to(cwd)),
        *sys.argv[3:],
    ]

    print(f"[{counter}/{len(runs)}]", *base_cmd, "...")
    proc = subprocess.run(
        [*base_cmd, *run.scope],
        shell=False,
    )
    if proc.returncode:
        raise SystemExit(1)
