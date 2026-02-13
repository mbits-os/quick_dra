# Copyright (c) 2026 Marcin Zdun
# This code is licensed under MIT license (see LICENSE for details)

# pylint: disable=locally-disabled, import-error

from typing import Any, cast

from proj_flow.api import env, step
from proj_flow.ext.cplusplus.cmake.steps import CMakeTest


@step.register(replace=True)
class CoverallsTest(CMakeTest):
    """Runs tests in the project using coveralls target."""

    def run(self, config: env.Config, rt: env.Runtime) -> int:
        if config.items.get("coverage"):
            cfg_test = cast(dict[str, Any], rt._cfg.get("test", {}))
            cfg_test_target = cast(str | None, cfg_test.get("target"))
            if cfg_test_target:
                return rt.cmd(
                    "cmake",
                    "--build",
                    "--preset",
                    config.preset,
                    "--target",
                    cfg_test_target,
                )
        return rt.cmd("ctest", "--preset", config.preset)
