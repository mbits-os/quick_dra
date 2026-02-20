# Copyright (c) 2026 Marcin Zdun
# This code is licensed under MIT license (see LICENSE for details)

# pylint: disable=locally-disabled, import-error

"""
The **quick_dra.test_coverage.coverage** provides coverage report loader,
returning ``io.CommentContext`` instance.
"""

import json
import sys
from pathlib import Path
from typing import Any, cast

from proj_flow.api import env
from quick_dra.test_coverage.io import (
    CommentContext,
    FileCoverage,
    from_data_list,
    mapped,
)


def _load_single_report(filename: Path):
    cvg = cast(dict[str, Any], json.loads(filename.read_bytes()))
    tag = cast(str | None, cvg.get("flag_name"))
    if not tag:
        tag = filename.stem

    files = mapped(
        lambda value: value.name,
        from_data_list(FileCoverage, cvg.get("source_files", [])),
    )

    return tag, files


def load_report(path: Path, rt: env.Runtime):
    """Load partial coverage reports from given file/directory."""
    try:
        result = CommentContext()

        if path.is_dir():
            paths: list[Path] = []
            for root, _, filenames in path.walk():
                paths.extend(
                    root / filename
                    for filename in filenames
                    if filename.endswith(".json")
                )
            for child in paths:
                result.update(*_load_single_report(child))
        else:
            result.update(*_load_single_report(path))

        return result
    except FileNotFoundError as e:
        rt.fatal(f"Cannot read '{e.filename}' file: {e.strerror}")
        sys.exit(1)
