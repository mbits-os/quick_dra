# Copyright (c) 2026 Marcin Zdun
# This code is licensed under MIT license (see LICENSE for details)

# pylint: disable=locally-disabled, import-error, missing-function-docstring

"""The ``quick_dra.icons`` extension adds Icons steps to the flow"""

import os
from pathlib import Path
from typing import cast

from proj_flow.api import env, step


@step.register
class QTestToJUnit(step.Step):
    """Converts QTest XML reports to JUnit file."""

    @property
    def name(self) -> str:
        """:meta private:"""
        return "QTestToJUnit"

    @property
    def runs_after(self) -> list[str]:
        """:meta private:"""
        return ["Test"]

    @property
    def runs_before(self) -> list[str]:
        """:meta private:"""
        return ["JUnitToCtrf"]

    def run(self, config: env.Config, rt: env.Runtime) -> int:
        build_dir = config.build_dir
        input_dir = build_dir / "test-results" / "qt-test"
        path_set = set[Path]()

        for cwd, _, files in input_dir.walk():
            path_set = path_set.union(
                set(cwd / file for file in files if file.endswith(".qtest.xml"))
            )

        inputs = [str(p) for p in sorted(path_set)]
        if not inputs:
            rt.message(
                f"No files found in {input_dir}",
                level=env.Msg.ALWAYS,
            )
            return 0
        output: Path | None = None

        test_cg = cast(dict, rt.items.get("test", {}))
        for config_key in ["gtest-dir", "junit-dir", "junit-dirs"]:
            config_name = cast(str | list[str] | None, test_cg.get(config_key))
            if config_name is None:
                continue
            output = Path(
                config_name if isinstance(config_name, str) else config_name[0]
            )
        if output is None:
            rt.message(
                'Cannot find the output directory inside "test" flow config',
                level=env.Msg.ALWAYS,
            )
            return 1

        binary = "qtxml-to-junit"
        if os.name == "nt":
            binary += ".exe"
        executable = build_dir / "tools" / binary

        rt.cmd(str(executable), *inputs, "-o", str(build_dir / output / "qt.xml"))
        return 0
