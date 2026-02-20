# Copyright (c) 2026 Marcin Zdun
# This code is licensed under MIT license (see LICENSE for details)

# pylint: disable=locally-disabled, import-error

"""
The **quick_dra.test_coverage** replaces the ``"Test"`` step and introduces
``coveralls`` and ``github comments`` command.

The new step will fall back on already registered ``"Test"`` step, if coverage
set to ``"OFF"`` in run matrix or if ``"test.target"`` is not set in Flow config.

The ``coveralls`` command is able to upload a single parallel build report and
set a "done" state for given parallel Coveralls build.

The ``github comments`` does some house-keeping with ever-growing number of
bot-made issue comments.
"""

from quick_dra.test_coverage import steps
from quick_dra.test_coverage.cli import comments, coveralls

__all__ = ["comments", "coveralls", "steps"]
