from dataclasses import dataclass, field
from typing import TextIO

from ..model import *
from .common import *
from .tmplt import TemplateContext


@dataclass
class Format:
    optional: str
    sequence: str
    union: str
    record: str
    generic: str


type_decorators: dict[str, Format] = {
    "cxx": Format(
        optional="std::optional<{}>",
        sequence="std::vector<{}>",
        union="std::variant<{}>",
        record="std::map<{}>",
        generic="{name}<{args}>",
    ),
    "py3": Format(
        optional="{} | None",
        sequence="list[{}]",
        union="Union[{}]",
        record="dict[{}]",
        generic="{name}[{args}]",
    ),
}


class TypeNames(TypeVisitor[str]):
    def __init__(self, lang: str, current_ext_attrs: list[dict[str, WidlExtAttribute]]):
        super(TypeVisitor).__init__()
        self.lang = lang
        self.decorators = type_decorators[lang]
        self.current_ext_attrs = current_ext_attrs

    def on_optional(self, obj: WidlOptional):
        args = ", ".join(self.on_subtypes(obj))
        return self.decorators.optional.format(args)

    def on_sequence(self, obj: WidlSequence):
        args = ", ".join(self.on_subtypes(obj))
        return self.decorators.sequence.format(args)

    def on_union(self, obj: WidlUnion):
        args = ", ".join(self.on_subtypes(obj))
        return self.decorators.union.format(args)

    def on_record(self, obj: WidlRecord):
        args = ", ".join(self.on_subtypes(obj))
        return self.decorators.record.format(args)

    def on_generic(self, obj: WidlGeneric):
        raw_args = self.on_subtypes(obj)
        class_name = generic_types[obj.name][self.lang][1]
        if class_name == "std::span" and raw_args:
            raw_args[0] += " const"
        args = ", ".join(raw_args)
        return self.decorators.generic.format(name=class_name, args=args)

    def on_simple(self, obj: WidlSimple):
        try:
            return simple_types[obj.text][self.lang][1]
        except KeyError:
            return obj.text


def _type_name(
    widl: WidlType, lang: str, current_ext_attrs: list[dict[str, WidlExtAttribute]]
):
    return widl.on_type_visitor(TypeNames(lang, current_ext_attrs))


class EnumValue:
    def __init__(self, value: str):
        self.name = f'"{value}"'
        self.safe = value.replace(",", "_").replace("-", "_")


class EnumInfo:
    def __init__(self, name: str, items: list[str], ext_attrs: dict):
        self.name = name
        self.NAME = name.upper()
        self.item = [EnumValue(item) for item in items]
        self.ext_attrs = ext_attrs

    @property
    def text(self):
        lines: list[str] = [
            f"#define {self.NAME}_X(X)",
            *[f"    X({item.safe}, {item.name})" for item in self.item[:-1]],
        ]
        max_len = max(len(line) for line in lines)
        text = []
        for line in lines:
            text.append(f"{line:<{max_len}} \\")
        text.append(f"    X({self.item[-1].safe}, {self.item[-1].name})")
        return "\n".join(text)


class AttributeInfo:
    def __init__(
        self,
        name: str,
        type_name: str,
        default: str,
        guards: list[str],
        ext_attrs: dict,
        for_variant: bool = False,
    ):
        self.name = name
        self.key = name.replace("_", "-")
        self.type = type_name
        self.default = f"{{{default}}}"
        self.guards = guards
        self.ext_attrs = ext_attrs
        self.for_variant = for_variant


class ArgumentInfo:
    def __init__(self, name: str, type_name: str, ext_attrs: dict):
        self.name = name
        self.type = type_name
        self.ext_attrs = ext_attrs
        self.comma = True


class OperationInfo:
    def __init__(
        self,
        name: str,
        type_name: str,
        guards: list[str],
        ext_attrs: dict,
        args: list[ArgumentInfo],
    ):
        self.name = name
        self.type = type_name
        self.guards = guards
        self.ext_attrs = ext_attrs
        self.args = args
        if self.args:
            self.args[-1].comma = False


@dataclass
class NestedInterfaceInfo:
    name: str
    expr: str
    attributes: list[AttributeInfo]


@dataclass
class InterfaceInfo:
    name: str
    ext_attrs: dict
    attributes: list[AttributeInfo]
    operations: list[OperationInfo]
    inheritance: Optional[str] = None
    subtypes: list[NestedInterfaceInfo] = field(default_factory=list)

    @property
    def spcs(self):
        return " " * len(self.name)


class CppConfigContext(TemplateContext):
    def __init__(self, output: TextIO, version: int, initial_context: dict):
        super().__init__(output, version)
        self.update(initial_context)
        self.includes: list[str] = []
        self.enums: list[EnumInfo] = []
        self.interfaces: list[InterfaceInfo] = []


class Visitor(TypeVisitor, ClassVisitor):
    def __init__(self, lang: str, project_types: list[str], ctx: CppConfigContext):
        super(TypeVisitor).__init__()
        super(ClassVisitor).__init__()
        self.lang = lang
        self.files: set[str] = set()
        self.project_types = project_types
        self.ctx = ctx
        self.current_ext_attrs: list[dict[str, WidlExtAttribute]] = []

    def all_visited(self):
        self.ctx.includes = list(sorted({*self.files}))

    def on_optional(self, obj: WidlOptional):
        self.on_subtype(obj)
        if self.lang == "cxx":
            self.files.add("<optional>")

    def on_sequence(self, obj: WidlSequence):
        self.on_subtype(obj)
        if self.lang != "cxx":
            return
        is_unique: WidlExtAttribute | None = None
        is_span: WidlExtAttribute | None = None
        for ext_attrs in self.current_ext_attrs:
            is_unique = ext_attrs.get("unique", is_unique)
            is_span = ext_attrs.get("span", is_span)
            if is_unique is not None and is_span is not None:
                break
        self.files.add("<span>" if is_span else "<set>" if is_unique else "<vector>")

    def on_union(self, obj: WidlUnion):
        self.on_subtype(obj)
        if self.lang == "cxx":
            self.files.add("<variant>")

    def on_record(self, obj: WidlRecord):
        self.on_complex(obj)
        if self.lang == "cxx":
            self.files.add("<map>")

    def on_simple(self, obj: WidlSimple):
        try:
            info = simple_types[obj.text][self.lang]
            if info[0] is not None:
                self.files.add(info[0])
        except KeyError:
            if obj.text in self.project_types:
                return
            print(f"unknown type: {obj.text}")

    def on_generic(self, obj: WidlGeneric):
        try:
            info = generic_types[obj.name][self.lang]
            if info[0] is not None:
                self.files.add(info[0])
        except KeyError:
            print(f"unknown type: {obj.name}{'<>' if self.lang == 'cxx' else '[]'}")

    def repack_props(self, props: list[WidlAttribute]):
        attributes: list[AttributeInfo] = []
        for prop in props:
            self.current_ext_attrs.append(prop.ext_attrs)
            prop.type.on_type_visitor(self)
            self.current_ext_attrs.pop()

            guard, guards = (prop.ext_attrs["guard"], prop.ext_attrs["guards"])
            if guard is not None:
                guards.append(guard)
            attributes.append(
                AttributeInfo(
                    prop.name,
                    _type_name(prop.type, self.lang, self.current_ext_attrs),
                    prop.ext_attrs["default"],
                    [*guards],
                    prop.ext_attrs,
                )
            )
        return attributes

    def on_interface(self, obj: WidlInterface):
        operations = self._initial_ops(obj)

        props, groups = obj.split_variants()

        attributes = self.repack_props(props)

        variant_type: list[str] = []
        variants: list[NestedInterfaceInfo] = []
        for group in groups:
            variant_type.append(group.name)
            variants.append(
                NestedInterfaceInfo(
                    group.name,
                    group.expr,
                    self.repack_props(group.props),
                )
            )

        if variant_type:
            self.files.add("<variant>")
            attributes.append(
                AttributeInfo(
                    "var",
                    f"std::variant<{', '.join(variant_type)}>",
                    "",
                    [],
                    {},
                    for_variant=True,
                )
            )

        for op in obj.ops:
            args: list[ArgumentInfo] = []

            self.current_ext_attrs.append(op.ext_attrs)
            op.type.on_type_visitor(self)
            for arg in op.args:
                self.current_ext_attrs.append(arg.ext_attrs)
                arg.type.on_type_visitor(self)
                args.append(
                    ArgumentInfo(
                        arg.name,
                        _type_name(arg.type, self.lang, self.current_ext_attrs),
                        arg.ext_attrs,
                    )
                )
                self.current_ext_attrs.pop()

            guard, guards = (op.ext_attrs["guard"], op.ext_attrs["guards"])
            if guard is not None:
                guards.append(guard)
            operations.append(
                OperationInfo(
                    op.name,
                    _type_name(op.type, self.lang, self.current_ext_attrs),
                    [*guards],
                    op.ext_attrs,
                    args,
                )
            )
            self.current_ext_attrs.pop()

        self.ctx.interfaces.append(
            InterfaceInfo(
                obj.name,
                ext_attrs=obj.ext_attrs,
                attributes=attributes,
                operations=operations,
                inheritance=obj.inheritance,
                subtypes=variants,
            )
        )

    def _initial_ops(self, obj: WidlInterface) -> list[OperationInfo]:
        initial: list[OperationInfo] = []

        no_spaceship = obj.ext_attrs.get("no_spaceship", False)
        comp_type, comp_op = ("bool", "==") if no_spaceship else ("auto", "<=>")

        initial.append(
            OperationInfo(
                f"operator{comp_op}",
                comp_type,
                [],
                {"defaulted": True},
                [ArgumentInfo("rhs", obj.name, {"in": True})],
            )
        )

        return initial

    def on_enum(self, obj: WidlEnum):
        self.ctx.enums.append(EnumInfo(obj.name, obj.items, obj.ext_attrs))


def print_from_template(
    objects: list[WidlClass],
    output: TextIO,
    version: int,
    print_lang: str,
    initial_context: dict,
    mustache_path: str,
    debug: bool = False,
):
    ctx = CppConfigContext(output, version, initial_context)
    Visitor(print_lang, [obj.name for obj in objects], ctx).visit_all(objects)
    ctx.emit_absolute(mustache_path, debug=debug)
