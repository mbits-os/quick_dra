# Copyright (c) 2026 Marcin Zdun
# This code is licensed under MIT license (see LICENSE for details)

# pylint: disable=locally-disabled, import-error

"""
The **quick_dra.test_coverage** replaces the ``"Test"`` step and introduces ``coveralls`` command.

The new step will fall back on already registered ``"Test"`` step, if coverage
set to ``"OFF"`` in run matrix or if ``"test.target"`` is not set in Flow config.

The ``coveralls`` command will set a "done" state for given parallel Coveralls build.
"""

import json
import os
import sys
from typing import Annotated, Any, cast
from urllib.parse import urlencode

from proj_flow.api import arg, env, step

ENDPOINT = "https://coveralls.io"

PREVIOUS_TEST = step.get_registered("Test")


@step.register(replace=True)
class CoverallsTest(step.Step):
    """Runs tests in the project using coveralls target."""

    @property
    def name(self):
        """:meta private:"""
        return "Test"

    @property
    def runs_after(self):
        """:meta private:"""
        return ["Build"]

    def is_active(self, config: env.Config, rt: env.Runtime) -> bool:
        """Checks with previous Test task, if it can run."""
        return PREVIOUS_TEST.is_active(config, rt) if PREVIOUS_TEST else False

    def platform_dependencies(self):
        """Checks with previous Test task, what are the dependencies"""
        return PREVIOUS_TEST.platform_dependencies() if PREVIOUS_TEST else []

    def directories_to_remove(self, config: env.Config):
        """Checks with previous Test task, which directories to remove"""
        return PREVIOUS_TEST.directories_to_remove(config) if PREVIOUS_TEST else []

    def run(self, config: env.Config, rt: env.Runtime) -> int:
        """Performs the task, falling back on previous Test task if necessary"""
        if config.items.get("coverage"):
            cfg_test = cast(dict[str, Any], rt.items.get("test", {}))
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
        if PREVIOUS_TEST:
            return PREVIOUS_TEST.run(config, rt)
        return 1


@arg.command("coveralls")
def coveralls(
    build_num: Annotated[
        str | None,
        arg.Argument(
            help="Finish the Coveralls parallel job", names=["--done"], opt=True
        ),
    ],
    rt: env.Runtime,
):
    """Read all Coveralls artifacts, merge them into one report and send to the service."""

    if not build_num:
        print("Only --done is currently implemented.")
        return 0

    repo_token = os.environ.get("COVERALLS_REPO_TOKEN")
    if not repo_token:
        print(
            "The $COVERALLS_REPO_TOKEN environment variable is missing. Not sending anything.",
            file=sys.stderr,
        )
        return 1

    url = f"{ENDPOINT}/webhook?{urlencode({"repo_token": repo_token})}"

    payload = {"payload": {"build_num": build_num, "status": "done"}}

    return rt.cmd(
        "curl",
        "-X",
        "POST",
        "-H",
        "Content-Type: application/json",
        "-d",
        json.dumps(payload, ensure_ascii=False),
        url,
    )
