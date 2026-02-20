# Copyright (c) 2026 Marcin Zdun
# This code is licensed under MIT license (see LICENSE for details)

# pylint: disable=locally-disabled, import-error

"""
The **quick_dra.test_coverage.cli.comments** introduces ``github comments``
command. This command does some house-keeping with ever-growing number of
bot-made issue comments.
"""

import os
import re
import sys
from typing import Annotated, Iterable

from proj_flow.api import arg, env
from quick_dra.test_coverage.github import (
    CommentInfo,
    filter_comments,
    find_comments,
    get_comment,
    github_delete_comments,
    github_get_issue_comments_per_url,
)


def __clean_duplicates(to_remove: list[str], unfiltered: Iterable[CommentInfo]):
    comments: list[CommentInfo] = []
    for comment in unfiltered:
        if not comments:
            comments.append(comment)
            continue

        last = comments[-1]
        if last.user_id != comment.user_id or last.body != comment.body:
            comments.append(comment)
            continue

        if last.url:
            to_remove.append(last.url)
        comments[-1] = comment  # replace with older one

    return comments


def __clean_old_test_summaries(current_run: int | None):
    def _dummy_filter(to_remove: list[str], unfiltered: Iterable[CommentInfo]):
        return unfiltered

    def _filter(body: str):
        is_summary = body.startswith("### Summary\n")
        has_test_results = body.startswith("### Test Results\n")
        if not is_summary and not has_test_results:
            return True

        tmplt = f"\n### build: [Run #{current_run}](https://"
        regex = r"(\n### build: \[Run #\d+\]\(https://)"
        m = re.search(regex, body)
        if m and m.group(0) != tmplt:
            return False

        return True

    if current_run is None:
        return _dummy_filter

    return find_comments(_filter)


@arg.command("github", "comments")
def coveralls(
    url: Annotated[
        str | None,
        arg.Argument(help="Check the issue or PR for doubling comments", opt=True),
    ],
    current_run: Annotated[
        int | None,
        arg.Argument(
            help="Check the issue or PR for doubling comments",
            meta="NUM",
            names=["--current-run"],
            opt=True,
        ),
    ],
    rt: env.Runtime,
):
    """Manage the list of bot-made comments."""

    if os.name == "nt":
        sys.stdout.reconfigure(encoding="utf-8")  # type: ignore

    data = github_get_issue_comments_per_url(url)

    to_remove: list[str] = []
    filter_comments(
        __clean_duplicates,
        __clean_old_test_summaries(current_run),
    )(to_remove, sorted(map(get_comment, data)))

    # pprint(comments)
    github_delete_comments(to_remove)
    print(f"Removed {len(to_remove)} comment{'' if len(to_remove) == 1 else 's'}")
