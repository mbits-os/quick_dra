# Copyright (c) 2026 Marcin Zdun
# This code is licensed under MIT license (see LICENSE for details)

# pylint: disable=locally-disabled, import-error, missing-function-docstring

"""The ``quick_dra.icons`` extension adds Icons steps to the flow"""

import os
import shutil
from pathlib import Path
from typing import Iterable, cast

from proj_flow.api import env, step


@step.register
class ImageErrors(step.Step):
    """Converts QTest XML reports to JUnit file."""

    @property
    def name(self) -> str:
        """:meta private:"""
        return "ImageErrors"

    @property
    def runs_after(self) -> list[str]:
        """:meta private:"""
        return ["Test"]

    def run(self, config: env.Config, rt: env.Runtime) -> int:
        build_dir = config.build_dir
        artifacts = build_dir.parent / "artifacts" / "test-results" / "images"
        binaries = cast(dict[str, str], rt.items.get("image-errors", {}))
        for binary, image_dir in binaries.items():
            if os.name == "nt":
                binary += ".exe"
            executable = build_dir / "bin" / binary

            error_files, replaced = _locate_errors(Path(image_dir))
            if not error_files:
                continue
            _rerun(rt, str(executable), replaced)
            _copy_to_artifacts(artifacts, error_files.union(replaced))

        return 0


def _locate_errors(image_dir: Path):
    error_files = set[Path]()
    for cwd, _, files in Path(image_dir).walk():
        error_files = error_files.union(
            cwd / filename for filename in files if filename.endswith(".error.png")
        )
    replaced = [
        filename.with_name(filename.name[: -len("error.png")] + "png")
        for filename in error_files
    ]

    return error_files, replaced


def _rerun(rt: env.Runtime, executable: str, replaced: list[Path]):
    for replacement in replaced:
        try:
            os.remove(replacement)
        except FileNotFoundError:
            # file will be recreated in a moment anyway
            pass
    dry_run = rt.dry_run
    rt.dry_run = False
    try:
        rt.cmd(executable)
    except SystemExit:
        # stay put
        pass
    rt.dry_run = dry_run


def _copy_to_artifacts(artifacts: Path, images: Iterable[Path]):
    for filename in images:
        target = artifacts / filename.parent
        target.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(filename, target / filename.name)
