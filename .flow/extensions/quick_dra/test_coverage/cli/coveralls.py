# Copyright (c) 2026 Marcin Zdun
# This code is licensed under MIT license (see LICENSE for details)

# pylint: disable=locally-disabled, import-error

"""
The **quick_dra.test_coverage.cli.coveralls** introduces ``coveralls`` command.

The ``coveralls`` command is able to upload a signle parallel build report and
set a "done" state for given parallel Coveralls build.
"""

import json
import os
import sys
from pathlib import Path
from typing import Annotated, cast
from urllib.parse import urlencode

from proj_flow.api import arg, env
from quick_dra.test_coverage.coverage import load_report
from quick_dra.test_coverage.github import (
    find_comments_starting_with,
    get_comment,
    github_create_comment,
    github_delete_comments,
    github_get_issue_comments_per_url,
    github_pr_new_code,
)
from quick_dra.test_coverage.io import CommentContext

ENDPOINT = "https://coveralls.io"


def _clean_old_comments(rt: env.Runtime):
    data = github_get_issue_comments_per_url(None)

    to_remove: list[str] = []
    find_comments_starting_with(
        "### Debug, ubuntu-",
        "### Debug, windows-",
        "'<!-- coverage-report:",
    )(
        to_remove,
        map(get_comment, data),
    )

    github_delete_comments(to_remove)
    print(f"Removed {len(to_remove)} comment{'' if len(to_remove) == 1 else 's'}")


def _comment_on_coverage(upload: str | None, build_num: str | None, rt: env.Runtime):
    report_path: Path | None = None
    if upload:
        report_path = Path(upload)
    elif build_num:
        report_path = Path("build", "artifacts", "coverage")

    if report_path:
        context = load_report(report_path, rt)
        ctx = context.mustache_context(github_pr_new_code())
        print(CommentContext.render_terminal(ctx), file=sys.stderr)
        github_create_comment(CommentContext.render_html(ctx))


def _finalize_parallel_report(build_num: str, repo_token: str | None, rt: env.Runtime):
    if not repo_token:
        return 0

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


def _upload_parallel_report(output: str, repo_token: str | None, rt: env.Runtime):
    """Uploads a file to coveralls.io."""
    if not repo_token:
        return 0

    url = f"{ENDPOINT}/api/v1/jobs"
    return rt.cmd("curl", "-S", "-F", f"json_file=@{output}", url)


@arg.command("coveralls")
def coveralls(
    upload: Annotated[
        str | None,
        arg.Argument(
            help="Upload a Coveralls parallel job report", meta="<path>", opt=True
        ),
    ],
    build_num: Annotated[
        str | None,
        arg.Argument(
            help="Finish the Coveralls parallel job", names=["--done"], opt=True
        ),
    ],
    rt: env.Runtime,
):
    """Manage the Coveralls parallel build within a single build workflow."""

    if os.name == "nt":
        sys.stdout.reconfigure(encoding="utf-8")  # type: ignore

    if build_num:
        _clean_old_comments(rt)

    _comment_on_coverage(upload, build_num, rt)

    if not upload and not build_num:
        print("error: one of --done and --upload is required")
        return 1

    repo_token = os.environ.get("COVERALLS_REPO_TOKEN")
    if not repo_token:
        print(
            "The $COVERALLS_REPO_TOKEN environment variable is missing. Not sending anything.",
            file=sys.stderr,
        )
        return 0

    if build_num:
        return _finalize_parallel_report(build_num, repo_token, rt)

    return _upload_parallel_report(cast(str, upload), repo_token, rt)
