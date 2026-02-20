# Copyright (c) 2026 Marcin Zdun
# This code is licensed under MIT license (see LICENSE for details)

"""
The **quick_dra.test_coverage.github** provides pull request manipulation helpers.
"""

import json
import os
import subprocess
import sys
from datetime import datetime
from pathlib import Path
from typing import Callable, Iterable, NamedTuple, TypeAlias, cast

from quick_dra.test_coverage.io import Range


def _github_pull_request_url(name: str):
    path = os.environ.get("GITHUB_EVENT_PATH")
    if not path:
        return None
    data = json.loads(Path(path).read_bytes())
    pull_request = cast(dict, data.get("pull_request", {}))
    return cast(str | None, pull_request.get(name))


def _gh(api: str, /, body: str | None = None, method: str | None = None):
    args = [
        "gh",
        "api",
        "-H",
        "Accept: application/vnd.github+json",
        "-H",
        "X-GitHub-Api-Version: 2022-11-28",
        api,
    ]
    if method:
        args[2:2] = ["--method", method]
    if body:
        if not method:
            args[2:2] = ["--method", "POST"]
        args.extend(["-f", f"body={body}"])

    proc = subprocess.run(args, capture_output=True, check=False)

    if proc.returncode:
        if proc.stderr:
            sys.stderr.write(proc.stderr.decode())
        return None

    return proc.stdout


def github_create_comment(body: str):
    """Adds a new comment to a current PR. Requires ``permissions.pull-requests: write``."""
    url = _github_pull_request_url("comments_url")
    if not url:
        print(
            "The comments URL is missing in environment. Not adding comments.",
            file=sys.stderr,
        )
        return

    blob = _gh(url, body)
    if blob:
        sys.stdout.write(blob.decode())


def github_pr_new_code():
    """Locates ranges of added text in current PR."""
    url = _github_pull_request_url("url")
    if not url:
        return None
    blob = _gh(f"{url}/files")
    if not blob:
        return None

    additions: dict[str, list[Range]] = {}
    pr_files = cast(list[dict[str, str]], json.loads(blob))
    for pr_file in pr_files:
        filename = pr_file.get("filename")
        patch = pr_file.get("patch")
        if not filename or not patch:
            continue

        lines = patch.split("\n")
        current_line_no = 1
        file_ranges: list[Range] = []
        for line in lines:
            if line.startswith("@@ "):
                current_line_no = int(
                    line[3:].split(" @@")[0].split(" ")[-1].split(",")[0]
                )
                continue
            if line.startswith("-"):
                continue
            new_line = line.startswith("+")
            if new_line:
                Range.append(file_ranges, current_line_no)
            current_line_no += 1

        additions[filename] = file_ranges

    return additions


def __get_comment_page(url: str, page: int):
    blob = _gh(f"{url}?page={page}")
    return json.loads(blob) if blob else []


def github_get_issue_comments_per_url(url: str | None):
    """Retrieves all comments per issue, paging as needed."""
    if url is None:
        url = _github_pull_request_url("comments_url")
    if not url:
        print("Comments URL is missing", file=sys.stderr)
        return []

    print(f"Downloading comments using {url}", file=sys.stderr)
    data: list[dict] = []
    page_no = 1
    while True:
        print(f"- page #{page_no}", file=sys.stderr)
        page = __get_comment_page(url, page_no)
        page_no += 1
        if not page:
            break
        data.extend(page)

    return data


def github_delete_comments(urls: Iterable[str]):
    """Deletes issue comments by URLs. Requires ``permissions.pull-requests: write``."""
    for url in urls:
        _gh(url, method="DELETE")


class CommentInfo(NamedTuple):
    """Excerpt from GitHub Comment object, as needed for filtering"""

    user_id: int | None
    body: str | None
    updated_at: datetime | None
    url: str | None


def map_get(root: dict, *path: str):
    """Gets a (possibly nested) value from a dictionary, or None if not found"""

    ctx = root
    for step in path:
        if not isinstance(ctx, dict):
            return None
        if step not in ctx:
            return None
        ctx = ctx[step]
    return ctx


def opt_cast[T](_: type[T], val):
    """``typing.cast``, but union with None"""
    return cast(T | None, val)


def get_comment(data: dict):
    """Converts JSON dictionary into a ``CommentInfo`` value."""
    body = opt_cast(str, map_get(data, "body"))
    updated_at = opt_cast(str, map_get(data, "updated_at"))
    url = opt_cast(str, map_get(data, "url"))
    user_id = opt_cast(int, map_get(data, "user", "id"))
    return CommentInfo(
        user_id=user_id,
        body=body,
        updated_at=datetime.fromisoformat(updated_at) if updated_at else None,
        url=url,
    )


CommentFilter: TypeAlias = Callable[
    [list[str], Iterable[CommentInfo]], Iterable[CommentInfo]
]


def filter_comments(*pipeline: CommentFilter):
    """Packs series of comment filters into a single filter."""

    def _impl(
        to_remove: list[str],
        unfiltered: Iterable[CommentInfo],
    ) -> Iterable[CommentInfo]:
        comments = unfiltered
        for fltr in pipeline:
            comments = fltr(to_remove, comments)

        return comments

    return _impl


def find_comments(keep_those: Callable[[str], bool]):
    """
    Given the unfiltered list of comments and a filter on body, marks comments
    for deletion.
    """

    def _impl(
        to_remove: list[str],
        unfiltered: Iterable[CommentInfo],
    ) -> Iterable[CommentInfo]:
        for comment in unfiltered:
            if not comment.body or not comment.url:
                yield comment
                continue
            body = comment.body
            if "\r" in body:
                body = body.replace("\r", "")

            keep = keep_those(body)
            if keep:
                yield comment
            else:
                to_remove.append(comment.url)

    return _impl


def find_comments_starting_with(*prefixes: str):
    """
    Given the unfiltered list of comments and a list of comment beginnings, marks
    comments for deletion.
    """

    return find_comments(lambda body: any(map(body.startswith, prefixes)))
