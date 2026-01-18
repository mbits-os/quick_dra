# Copyright (c) 2026 Marcin Zdun
# This code is licensed under MIT license (see LICENSE for details)

from proj_flow.ext.webidl.model.ast import MergedDefinitions
from proj_flow.ext.webidl.registry import WebIDLVisitor, webidl_visitors


@webidl_visitors.add
class Visitor(WebIDLVisitor):
    def on_definitions(self, definitions: MergedDefinitions):
        for interface in definitions.interfaces.values():
            for attribute in interface.attributes:
                attribute.ext_attrs["yaml_key"] = attribute.name.replace("_", "-")
