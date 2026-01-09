# Copyright (c) 2025 midnightBITS
# This code is licensed under MIT license (see LICENSE for details)

import argparse
import copy
import json
import os
from contextlib import contextmanager
from pprint import pprint
from typing import cast

from widl.model import *
from widl.parser import parse_all
from widl.printers import print_from_template, render_text, reorder_types
from widl.printers.common import generic_types, simple_types
from widl.ver import VERSION


@contextmanager
def open_file(path: str):
    dirname = os.path.dirname(path)
    os.makedirs(dirname, exist_ok=True)
    with open(path, "w", encoding="UTF-8") as output:
        yield output


parser = argparse.ArgumentParser(description="Generate YAML model in C++")
parser.add_argument(
    "--version",
    metavar="<num>",
    default=VERSION,
    type=int,
    help=f"select the version for generated code; defaults to {VERSION}",
)
parser.add_argument(
    "--debug",
    default=False,
    action="store_true",
    help=f"print debug information",
)
parser.add_argument(
    "-d",
    dest="defines",
    metavar="<var>=<value>",
    action="append",
    help="declare variables for file config path templates",
    required=False,
)
parser.add_argument(
    "-c",
    dest="config",
    metavar="<json>",
    help="extend the IDL type system with aliases and attributes",
    required=True,
)
parser.add_argument(
    metavar="<idl>",
    nargs="+",
    dest="paths",
    help="generate code based on these definitions",
)
args = parser.parse_args()
config_dir = os.path.dirname(args.config)
defines = {
    val.split("=", 1)[0].strip(): val.split("=", 1)[1].strip()
    for val in cast(list[str], args.defines)
}

with open(args.config, encoding="UTF-8") as json_file:
    data = cast(dict, json.load(json_file))


@dataclass
class Output:
    output_name: str
    mustache_template: str | None
    output_template: str | None
    lang: str | None
    initial_context: dict

    @staticmethod
    def from_config(data: dict[str, str | dict[str, Any]]):
        result = [Output.from_dict(key, item) for key, item in data.items()]

        for index in range(len(result)):
            default = result[index]
            if default.output_name != "":
                continue

            del result[index]
            for item in result:
                item.update_from(default)

            break

        return result

    @staticmethod
    def from_dict(output_name: str, data: str | dict[str, str | dict]) -> "Output":
        if isinstance(data, str):
            return Output(
                output_name=output_name,
                mustache_template=data,
                output_template=None,
                lang=None,
                initial_context={},
            )

        mustache_template = cast(str | None, data.get("template", None))
        output_template = cast(str | None, data.get("path", None))
        lang = cast(str | None, data.get("lang", None))
        initial_context = cast(dict, data.get("context", {}))

        return Output(
            output_name=output_name,
            mustache_template=mustache_template,
            output_template=output_template,
            lang=lang,
            initial_context=initial_context,
        )

    def update_from(self, default: "Output"):
        self.mustache_template = self.mustache_template or default.mustache_template
        self.output_template = self.output_template or default.output_template
        self.lang = self.lang or default.lang
        self.initial_context = (
            self.initial_context
            if len(self.initial_context)
            else copy.deepcopy(default.initial_context)
        )

    def render(self, defines: dict, objects: list[WidlClass], debug: bool):
        if not self.mustache_template or not self.output_template:
            return

        vars = defines.copy()
        vars["PATH"] = self.output_name

        template_path = os.path.abspath(
            os.path.join(config_dir, self.mustache_template)
        )
        output_path = render_text(self.output_template, vars, debug=debug)

        print(template_path, '->', output_path)

        try:
            with open_file(output_path) as output_file:
                print_from_template(
                    objects,
                    output_file,
                    VERSION,
                    output.lang or "cxx",
                    self.initial_context,
                    template_path,
                    debug=debug,
                )
        except:
            try:
                os.unlink(output_path)
            except:
                pass

            raise


def add_types(data: dict, key: str, registry: dict[str, Any]):
    user_types = cast(dict[str, dict | list], data.get(key, {}))
    for name, decl in user_types.items():
        if name not in registry:
            registry[name] = {}
        if isinstance(decl, list):
            src, typename = decl
            registry[name]["cxx"] = (src, typename)
        else:
            print(user_types)
            print(name)
            print(decl)
            for lang, (src, typename) in decl.items():
                registry[name][lang] = (src, typename)


user_attrs = cast(
    dict[ExtAttrDom, dict[str, ExtAttrType]], data.get("ext-attributes", {})
)
for domain, attrs in user_attrs.items():
    install_user_attrs(domain, attrs)

add_types(data, "simple_types", simple_types)
add_types(data, "generic_types", generic_types)

try:
    objects = parse_all(args.paths)
except RuntimeError as e:
    print(e)
    raise SystemExit(1)

reorder_types(objects)

for output in Output.from_config(cast(dict[str, str | dict], data.get("mustache", {}))):
    output.render(defines, objects, debug=args.debug)
