# Copyright (c) 2026 Marcin Zdun
# This code is licensed under MIT license (see LICENSE for details)

# pylint: disable=locally-disabled, import-error

"""Extends ``proj_flow.ext.webidl.cli.gen`` for YAML reader generated from WebIDL"""

from proj_flow.ext.webidl.model.ast import MergedDefinitions
from proj_flow.ext.webidl.registry import WebIDLVisitor, webidl_visitors


@webidl_visitors.add
class Visitor(WebIDLVisitor):
    """WebIDL Visitor implementation"""

    def on_definitions(self, definitions: MergedDefinitions):
        """Replace underscore with dashes for unmodified property names"""

        for interface in definitions.interfaces.values():
            for attribute in interface.attributes:
                attribute.ext_attrs["yaml_key"] = attribute.name.replace("_", "-")
