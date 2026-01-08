from dataclasses import dataclass
from typing import Any, Callable, Optional, TypeVar, cast

from ..types import file_pos
from .WidlExtAttribute import (
    WidlExtAttribute,
    WidlExtendable,
    attribute_ext_attrs,
    enum_ext_attrs,
    interface_ext_attrs,
    oparg_ext_attrs,
    operation_ext_attrs,
)


class ClassVisitor:
    def __init__(self):
        pass

    def on_class(self, _: "WidlClass"):
        pass

    def on_enum(self, obj: "WidlEnum"):
        return self.on_class(obj)

    def on_interface(self, obj: "WidlInterface"):
        return self.on_class(obj)

    def visit_all(self, objects: list["WidlClass"]):
        for object in objects:
            object.on_class_visitor(self)
        self.all_visited()
        return self

    def visit_one(self, object: "WidlClass"):
        object.on_class_visitor(self)
        return self

    def all_visited(self):
        pass


def visit_all_classes(objects: list["WidlClass"], visitors: list[ClassVisitor]):
    for visitor in visitors:
        visitor.visit_all(objects)


T = TypeVar("T")


class TypeVisitor[T]:
    def __init__(self):
        pass

    def on_type(self, _: "WidlType") -> T: ...

    def on_simple(self, obj: "WidlSimple"):
        return self.on_type(obj)

    def on_subtype(self, obj: "WidlComplex"):
        for sub in obj.sub:
            sub.on_type_visitor(self)
        return cast(T, None)

    def on_subtypes(self, obj: "WidlComplex") -> list[T]:
        return [sub.on_type_visitor(self) for sub in obj.sub]

    def on_complex(self, obj: "WidlComplex"):
        return self.on_subtype(obj)

    def on_sequence(self, obj: "WidlSequence"):
        return self.on_complex(obj)

    def on_union(self, obj: "WidlUnion"):
        return self.on_complex(obj)

    def on_record(self, obj: "WidlRecord"):
        return self.on_complex(obj)

    def on_generic(self, obj: "WidlGeneric"):
        return self.on_complex(obj)

    def on_optional(self, obj: "WidlOptional"):
        return self.on_complex(obj)


class WidlClass(WidlExtendable):
    def __init__(
        self,
        type: str,
        name: str,
        ext_attrs: list[WidlExtAttribute],
        transform: Callable[[list[WidlExtAttribute]], dict],
        pos: file_pos | None,
    ):
        super().__init__(ext_attrs, transform, pos)
        self.type = type
        self.name = name
        self.partial = False

    def on_class_visitor(self, visitor: ClassVisitor):
        pass


class WidlEnum(WidlClass):
    def __init__(
        self,
        name: str,
        items: list[str],
        ext_attrs: list[WidlExtAttribute],
        pos: file_pos | None,
    ):
        super().__init__("enum", name, ext_attrs, enum_ext_attrs, pos)
        self.items = items

    def __str__(self):
        return "enum {} ({})".format(self.name, ", ".join(self.items))

    def on_class_visitor(self, visitor: ClassVisitor):
        return visitor.on_enum(self)


class WidlType:
    def __init__(self):
        pass

    def on_type_visitor(self, visitor: TypeVisitor) -> Any:
        pass


class WidlSimple(WidlType):
    def __init__(self, text: str):
        super().__init__()
        self.text = text

    def __str__(self):
        return self.text

    def on_type_visitor(self, visitor: TypeVisitor):
        return visitor.on_simple(self)


class WidlComplex(WidlType):
    def __init__(self, *sub: WidlType):
        super().__init__()
        self.sub = sub

    def on_type_visitor(self, visitor: TypeVisitor):
        return visitor.on_complex(self)


class WidlSequence(WidlComplex):
    def __str__(self):
        return f"{self.sub}[]"

    def on_type_visitor(self, visitor: TypeVisitor):
        return visitor.on_sequence(self)

class WidlUnion(WidlComplex):
    def __str__(self):
        return f"{self.sub}[]"

    def on_type_visitor(self, visitor: TypeVisitor):
        return visitor.on_union(self)


class WidlRecord(WidlComplex):
    def __init__(self, key: WidlType, value: WidlType):
        super().__init__(key, value)

    def __str__(self):
        return f"{{[{self.sub[0]}]: {self.sub[1]}}}"

    def on_type_visitor(self, visitor: TypeVisitor):
        return visitor.on_record(self)


class WidlGeneric(WidlComplex):
    def __init__(self, name: str, *sub: WidlType):
        super().__init__(*sub)
        self.name = name

    def __repr__(self):
        return str(self)

    def __str__(self):
        sub = ", ".join(str(s) for s in self.sub)
        return f"{self.name}<{sub}>"

    def on_type_visitor(self, visitor: TypeVisitor):
        return visitor.on_generic(self)


class WidlOptional(WidlComplex):
    def __str__(self):
        return f"{self.sub[0]}?"

    def on_type_visitor(self, visitor: TypeVisitor):
        return visitor.on_optional(self)


class WidlAttribute(WidlExtendable):
    def __init__(
        self,
        name: str,
        type: WidlType,
        ext_attrs: list[WidlExtAttribute],
        pos: file_pos | None,
    ):
        super().__init__(ext_attrs, attribute_ext_attrs, pos)
        self.name = name
        self.type = type

    def __str__(self):
        return f"{self.type} {self.name};"


class WidlArgument(WidlExtendable):
    def __init__(
        self,
        name: str,
        type: WidlType,
        ext_attrs: list[WidlExtAttribute],
        pos: file_pos | None,
    ):
        super().__init__(ext_attrs, oparg_ext_attrs, pos)
        self.name = name
        self.type = type


class WidlOperation(WidlExtendable):
    def __init__(
        self,
        name: str,
        type: WidlType,
        args: list[WidlArgument],
        ext_attrs: list[WidlExtAttribute],
        pos: file_pos | None,
    ):
        super().__init__(ext_attrs, operation_ext_attrs, pos)
        self.name = name
        self.type = type
        self.args = args


@dataclass
class WidlAttributeGroup:
    name: str
    expr: str
    props: list[WidlAttribute]


class WidlInterface(WidlClass):
    def __init__(
        self,
        name: str,
        props: list[WidlAttribute],
        ops: list[WidlOperation],
        ext_attrs: list[WidlExtAttribute],
        inheritance: Optional[str],
        pos: file_pos | None,
    ):
        super().__init__("interface", name, ext_attrs, interface_ext_attrs, pos)
        self.inheritance = inheritance
        self.props = props
        self.ops = ops

    def __str__(self):
        if self.inheritance:
            return f"interface {self.name} : {self.inheritance} (...)"
        return f"interface {self.name} (...)"

    def on_class_visitor(self, visitor: ClassVisitor):
        return visitor.on_interface(self)

    def split_variants(self):
        variant_names: dict[str, str] = {}
        variant_props: dict[str, list[WidlAttribute]] = {}

        for prop in self.props:
            (if_attr, variant_name) = (
                prop.ext_attrs["if"] or "",
                prop.ext_attrs["var"],
            )
            if variant_name is not None and if_attr not in variant_names:
                variant_names[if_attr] = variant_name
            try:
                variant_props[if_attr].append(prop)
            except KeyError:
                variant_props[if_attr] = [prop]

        free_props = variant_props.get("")
        if free_props is not None:
            del variant_props[""]

        groups: list[WidlAttributeGroup] = []
        counter = 0
        for if_attr in sorted(variant_props.keys()):
            variant_name = variant_names.get(if_attr)
            if variant_name is None:
                variant_name = f"variant_{counter}"
                counter += 1
            groups.append(
                WidlAttributeGroup(variant_name, if_attr, variant_props[if_attr])
            )

        return (free_props or [], groups)


class VisitAllTypes(ClassVisitor):
    def __init__(self, visitor: TypeVisitor):
        self.visitor = visitor

    def visit(self, type: WidlType):
        type.on_type_visitor(self.visitor)

    def on_interface(self, obj: WidlInterface):
        for prop in obj.props:
            self.visit(prop.type)

        for op in obj.ops:
            self.visit(op.type)

            for arg in op.args:
                self.visit(arg.type)
