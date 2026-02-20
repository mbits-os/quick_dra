# Copyright (c) 2026 Marcin Zdun
# This code is licensed under MIT license (see LICENSE for details)

# pylint: disable=locally-disabled, import-error

"""
The **quick_dra.test_coverage.io** provides coverage data types.
"""

from dataclasses import Field, asdict, dataclass, field, fields
from typing import Any, Callable, ClassVar, Iterable, NamedTuple, Protocol, cast

import chevron

EMOJIS = {
    "bad": "⛔",
    "warn": "⚠️",
    "good": "✅",
}

SHELL_COLORS = {
    "bad": "1;31",
    "warn": "1;33",
    "good": "1;32",
}


class DataclassInstance(Protocol):
    """
    DataclassInstance takes from the @dataclass protocol and adds ``from_data``.
    """

    __dataclass_fields__: ClassVar[dict[str, Field[Any]]]

    @classmethod
    def from_data(cls, **kwargs) -> "DataclassInstance":
        """
        The ``from_data`` each class modelling this protocol must provide.
        """
        ...  # pylint: disable=unnecessary-ellipsis


def from_data_impl[T: (DataclassInstance)](cls: type[T], **kwargs):
    """
    An implementation helper for a ``cls.from_data`` class method.
    """
    keys = list(kwargs.keys())
    for f in fields(cls):
        if f.name in keys:
            keys.remove(f.name)
    for key in keys:
        del kwargs[key]
    return cls(**kwargs)


def from_data_list[T: (DataclassInstance)](cls: type[T], items: list[dict[str, Any]]):
    """
    Maps list of dictionary to an instance of ``cls``, using a ``from_data``
    class method.
    """
    return cast(list[T], list(map(lambda item: cls.from_data(**item), items)))


def mapped[T, K](key: Callable[[T], K], items: Iterable[T]):
    """
    Helper mapping a list of items into a dictionary, based on a key evaluated
    from each value (for instance, a member).
    """
    return {key(item): item for item in items}


@dataclass
class Range:
    """Line range in a source file."""

    start: int
    stop: int

    @staticmethod
    def append(ranges: "list[Range]", line_no: int):
        """Extends the ranges with current line number."""
        if ranges and ranges[-1].stop + 1 == line_no:
            ranges[-1].stop += 1
        else:
            ranges.append(Range(line_no, line_no))

    def contains(self, line_no: int):
        """Returns ``True``, if ``line_no`` is within this range edges, inclusive."""
        return self.start <= line_no <= self.stop

    @staticmethod
    def any_contains(ranges: "list[Range]", line_no: int):
        """
        Returns ``True``, if either ``ranges`` is empty, or ``line_no`` is
        within one of given ranges.
        """
        if not ranges:
            return True
        for one in ranges:
            if one.contains(line_no):
                return True
        return False


class Coverage(NamedTuple):
    """Value gathered along a LineCoverage"""

    relevant: int
    visited: int


class CoverageLine(NamedTuple):
    """Stats for a named coverage (e.g. Total, New code, etc.)"""

    label: str
    relevant: int
    visited: int

    @property
    def percent_f(self):
        """Returns calculated percentage for this coverage"""
        if self.relevant == 0:
            return 100.0
        return self.visited * 100 / self.relevant

    @property
    def percent(self):
        """Returns formatted percentage for this coverage"""
        return f"{self.percent_f:.2f}%"

    @property
    def status(self):
        """One of 'bad'/'warn'/'good'."""
        return "bad" if self.bad else "warn" if self.warn else "good"

    @property
    def bad(self):
        """Returns true for coverage below 75%"""
        return self.percent_f < 75

    @property
    def warn(self):
        """Returns true for coverage between 75% and 90%"""
        percent = self.percent_f
        return 75 <= percent < 90

    @property
    def good(self):
        """Returns true for coverage of 90% and above"""
        return 90 <= self.percent_f

    @property
    def emoji(self):
        """Emoji value for this coverage, one of ✅/⚠️/⛔."""
        return EMOJIS[self.status]

    @property
    def shell_color(self):
        """Shell escape value for this coverage."""
        return SHELL_COLORS[self.status]


class LineCoverage(list[int | None]):
    """
    Keeps a list of call counts for each line; lines which are not relevant
    are represented by a None.
    """

    def get_coverage(self, ranges: list[Range]):
        """Calculate the coverage falling under listed ranges"""
        relevant = 0
        visited = 0
        for index, data in enumerate(self):
            if data is None:
                continue
            if Range.any_contains(ranges, index + 1):
                relevant += 1
                if data:
                    visited += 1
        return Coverage(relevant=relevant, visited=visited)


def extract_kwarg(kwargs: dict[str, Any], key: str, default):
    """Get a named argument from a kwargs for further processing."""

    result = kwargs.get(key, default)
    if key in kwargs:
        del kwargs[key]
    return result


@dataclass
class FileCoverage:
    """Holds a line coverage for a given file."""

    name: str
    coverage: LineCoverage = field(default_factory=LineCoverage)

    @classmethod
    def from_data(cls, **kwargs):
        """
        Create and instance of ``LineCoverage`` from a dictionary,
        possibly from a JSON file.
        """
        extract_kwarg(kwargs, "functions", None)
        coverage = cast(list[int | None], extract_kwarg(kwargs, "coverage", []))
        return from_data_impl(
            cls,
            **kwargs,
            coverage=LineCoverage(coverage),
        )

    def merge(self, other: "FileCoverage"):
        """Adds ``other`` line coverage to this line coverage"""
        self.coverage = _merge_coverage(self.coverage, other.coverage)


def _merge_cvg_value(v: tuple[int | None, int | None]):
    left, right = v
    if left is None:
        return right
    if right is None:
        return left
    return left + right


def _merge_coverage(left: LineCoverage, right: LineCoverage) -> LineCoverage:
    line_count = max(len(left), len(right))
    left.extend([None] * (line_count - len(left)))
    right.extend([None] * (line_count - len(right)))
    return LineCoverage(map(_merge_cvg_value, zip(left, right)))


FileCoverages = dict[str, FileCoverage]


@dataclass
class CommentContext:
    """
    Holds all partial file coverage reports, to later build a mustache report
    out of them.
    """

    combined: FileCoverages = field(default_factory=FileCoverages)
    tagged: dict[str, FileCoverages] = field(default_factory=dict)

    def update(self, tag: str, coverage: FileCoverages):
        """Records a partial coverage report in the coverage context"""
        for key, cvg in coverage.items():
            try:
                self.combined[key].merge(cvg)
            except KeyError:
                self.combined[key] = from_data_impl(FileCoverage, **asdict(cvg))
        self.tagged[tag] = coverage

    def unfiltered_ranges(self):
        """Builds a context filter taking in every known file"""
        full_range: dict[str, list[Range]] = {}
        for key in self.combined:
            full_range[key] = []
        return full_range

    def mustache_context(self, pr_filter: dict[str, list[Range]] | None):
        """Prepares the mustache context for gathered coverage info"""
        args = (
            ("Line coverage rate is", self.unfiltered_ranges()),
            ("New lines are covered at", pr_filter),
        )

        tags: list[dict] = []
        for key in sorted(self.tagged.keys()):
            lines = CommentContext._report_on(self.tagged[key], *args)
            if lines:
                tags.append({"tag": key, "lines": lines})

        title = "Coverage report"
        combined = CommentContext._report_on(self.combined, *args)
        comment_type = "combined"

        if len(tags) == 1:
            title = cast(str, tags[0].get("tag", title))
            tags = []
            comment_type = "partial"

        return {
            "title": title,
            "type": comment_type,
            "combined": {"lines": combined},
            "tags": tags,
            "has-tags": len(tags),
        }

    @staticmethod
    def render_html(ctx: dict):
        """Renders the context for HTML/MD"""
        return chevron.render(
            """<!-- coverage-report:{{type}} -->
### {{title}}
{{#combined}}{{> report}}{{/combined}}
{{#has-tags}}

<details>
<summary><b>Coverage for each build</b></summary>{{#tags}}{{> report}}{{/tags}}
</details>
{{/has-tags}}
""",
            data=ctx,
            partials_dict={
                "report": """
<p>{{#tag}}<b>{{.}}</b><br/>{{/tag}}{{#lines}}
{{label}} <b><span title="from {{visited}} lines visited and {{relevant}} lines relevant">{{!
    break for linters
}}{{percent}}</span></b> {{emoji}}<br/>{{/lines}}</p>""",
            },
        )

    @staticmethod
    def render_terminal(ctx: dict):
        """Renders the context for text terminal"""
        return chevron.render(
            """\033[1;97m{{title}}\033[m{{#combined}}{{> report}}{{/combined}}{{!
    break for linters
}}{{#tags}}{{> report}}{{/tags}}""",
            data=ctx,
            partials_dict={
                "report": """{{#tag}}

\033[0;34m{{.}}\033[m{{/tag}}{{#lines}}
  \033[0;90m{{label}} \033[{{shell_color}}m{{percent}}\033[m{{/lines}}
""",
            },
        )

    @staticmethod
    def _report_on_facet(cvg: FileCoverages, ranges: dict[str, list[Range]]):
        relevant = 0
        visited = 0
        for name, file_cvg in cvg.items():
            if name not in ranges:
                continue
            result = file_cvg.coverage.get_coverage(ranges[name])
            relevant += result.relevant
            visited += result.visited
        return Coverage(relevant, visited)

    @staticmethod
    def _report_on(
        cvg: FileCoverages, *args: tuple[str, dict[str, list[Range]] | None]
    ):
        result: list[CoverageLine] = []
        for label, ranges in args:
            if ranges is None:
                continue

            data = CommentContext._report_on_facet(cvg, ranges)
            if data.relevant == 0:
                continue

            result.append(CoverageLine(label, data.relevant, data.visited))
        return result
