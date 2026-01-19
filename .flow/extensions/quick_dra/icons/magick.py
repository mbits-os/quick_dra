# Copyright (c) 2026 Marcin Zdun
# This code is licensed under MIT license (see LICENSE for details)

# pylint: disable=locally-disabled, import-error, missing-function-docstring

"""Provides Makefile-like set of rules to build icons using ImageMagick"""

import os
import sys
from typing import List

from proj_flow.api.env import Runtime
from proj_flow.api.makefile import Rule, Statement

TOOL = "magick" if sys.platform == "win32" else "convert"


class MkDirs(Rule):
    """``MkDirs`` creates directories named by outputs"""

    def command(self, statement: Statement):
        return []

    def run(self, statement: Statement, rt: Runtime):
        for output in statement.outputs:
            result = rt.mkdirs(output)
            if result:
                return result
        return 0


class Copy(Rule):
    """``Copy`` copies first input into first output"""

    def command(self, statement: Statement):
        return []

    def run(self, statement: Statement, rt: Runtime):
        return rt.cp(statement.inputs[0], statement.outputs[0])


class Magick:
    """``Magick`` holds all ImageMagick rules"""

    class SvgToPng(Rule):
        """``Magick.Merge`` turns an SVG in first input into a PNG in first output"""

        def command(self, statement: Statement):
            return [
                TOOL,
                "-background",
                "none",
                statement.inputs[0],
                statement.outputs[0],
            ]

    class Resize(Rule):
        """``Magick.Resize`` resizes first input into first output"""

        size: str

        def __init__(self, size: str):
            self.size = size

        def command(self, statement: Statement):
            return [
                TOOL,
                "-background",
                "none",
                statement.inputs[0],
                "-resize",
                f"{self.size}x{self.size}",
                "-depth",
                "32",
                statement.outputs[0],
            ]

    class Merge(Rule):
        """``Magick.Merge`` merges all inputs into first output"""

        def command(self, statement: Statement):
            return [
                TOOL,
                *statement.inputs,
                statement.outputs[0],
            ]


def mkdirs(dirname: str):
    return MkDirs.statement([dirname], [])


def copy(src: str, dst: str):
    return Copy.statement([dst], [src])


def svg_to_png(output: str, image: str):
    return Magick.SvgToPng.statement([output], [image], [os.path.dirname(output)])


def resize(output: str, stencil: str, size: str):
    return Statement(
        Magick.Resize(size), [output], [stencil], [os.path.dirname(output)]
    )


def merge(output: str, layers: List[str]):
    return Magick.Merge.statement([output], layers, [os.path.dirname(output)])
