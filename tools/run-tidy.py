import json
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable, cast

import yaml

try:
    from yaml import CLoader as Loader
except ImportError:
    from yaml import Loader

cwd = Path().resolve()
src_roots = [cwd / "libs"]

clang_tidy_exec = sys.argv[1]
build_dir = sys.argv[2]


@dataclass
class TidyRun:
    config_file: str
    scope: list[str] = field(default_factory=list)


def _get_files():
    commands = json.loads(Path(build_dir, "compile_commands.json").read_bytes())
    return map(
        lambda command: Path(command.get("file", "")),
        cast(list[dict[str, str]], commands),
    )


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


for run in _get_runs(_get_files()).values():
    checks = (
        cast(
            dict[str, str], yaml.load(Path(run.config_file).read_bytes(), Loader=Loader)
        ).get("Checks")
        or "-*"
    )
    if checks == "-*":
        continue

    print(
        clang_tidy_exec,
        "-p",
        build_dir,
        "--quiet",
        "--use-color",
        "--config-file",
        run.config_file,
        *sys.argv[3:],
        "...",
    )

    proc = subprocess.run(
        [
            clang_tidy_exec,
            "-p",
            build_dir,
            "--quiet",
            "--use-color",
            "--config-file",
            run.config_file,
            *sys.argv[3:],
            *run.scope,
        ],
        shell=False,
    )
    if proc.returncode:
        raise SystemExit(1)
