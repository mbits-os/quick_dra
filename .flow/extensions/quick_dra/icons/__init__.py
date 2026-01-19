# Copyright (c) 2026 Marcin Zdun
# This code is licensed under MIT license (see LICENSE for details)

# pylint: disable=locally-disabled, import-error, missing-function-docstring

"""The ``quick_dra.icons`` extension adds Icons steps to the flow"""

import os
from typing import List

from proj_flow.api import env, step
from proj_flow.api.makefile import Makefile, Statement

from . import magick

ICONS = os.path.join("data", "icons")
ASSETS = os.path.join("data", "assets")
ICONS_PNG = os.path.join(ICONS, "png")
STENCIL = os.path.join(ICONS_PNG, "appicon.png")
SIZES = ["16", "24", "32", "48", "256"]

statements: List[Statement] = []

for size in SIZES:
    sized_image = os.path.join(ICONS, f"appicon-{size}.svg")

    if os.path.isfile(sized_image):
        statements.append(
            magick.svg_to_png(
                output=os.path.join(ICONS_PNG, f"appicon-{size}.png"),
                image=sized_image,
            )
        )
    else:
        statements.append(
            magick.resize(
                output=os.path.join(ICONS_PNG, f"appicon-{size}.png"),
                stencil=STENCIL,
                size=size,
            )
        )

makefile = Makefile(
    [
        magick.svg_to_png(
            output=STENCIL,
            image=os.path.join(ICONS, "appicon.svg"),
        ),
        *statements,
        magick.merge(
            os.path.join(ASSETS, "appicon.ico"),
            [os.path.join(ICONS_PNG, f"appicon-{size}.png") for size in SIZES],
        ),
        magick.mkdirs(ICONS_PNG),
        magick.copy(
            os.path.join(ICONS_PNG, "appicon-256.png"),
            os.path.join(ASSETS, "appicon.png"),
        ),
    ]
)


@step.register
class IconsStep:
    """Builds the application icon(s) from source SCG files"""

    name = "Icons"
    runs_before = ["Build"]

    def platform_dependencies(self):
        return [f"{magick.TOOL}>=6"]

    def run(self, config: env.Config, rt: env.Runtime) -> int:
        return makefile.run(rt)
