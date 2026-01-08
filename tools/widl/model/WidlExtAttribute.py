import sys
from enum import Enum
from typing import Callable

from ..types import file_pos, partial_token, token


class WidlExtAttribute:
    def __init__(self, name: token, arguments: list[partial_token]):
        self.name = name
        self.args = arguments

    def __str__(self):
        if len(self.args) == 0:
            return f"{self.name}"
        if len(self.args) == 1:
            return f"{self.name}={self.args[0]}"
        return "{}({})".format(self.name, ", ".join(str(arg) for arg in self.args))


class WidlExtendable:
    def __init__(
        self,
        ext_attrs: list[WidlExtAttribute],
        transform: Callable[[list[WidlExtAttribute]], dict],
        pos: file_pos | None,
    ):
        self.ext_attrs = transform(ext_attrs)
        self.pos = pos


def error(pos: file_pos | None, msg: str):
    if pos:
        print(f"{pos}: error: {msg}", file=sys.stderr)
    else:
        print(f"error: {msg}", file=sys.stderr)
    sys.exit(1)


def flag_attribute(name: str, attr: WidlExtAttribute):
    if len(attr.args) != 0:
        error(attr.name.pos, "`{}' has no arguments".format(name))
    return True


def single_arg_attribute(name: str, values: list[str], attr: WidlExtAttribute):
    if len(attr.args) != 1:
        error(attr.name.pos, "`{}' requires exactly 1 argument".format(name))
    if attr.args[0].code not in values:
        error(
            attr.name.pos,
            "`{}' needs to be one of: {}".format(
                name, ", ".join(f"`{value}'" for value in values)
            ),
        )
    return attr.args[0].code


class SingleArg:
    def __init__(self, name: str, values: list[str], default: str):
        self.name = name
        self.values = values
        self.default = default

    def __repr__(self):
        return (
            f"SingleArg({repr(self.name)}, {repr(self.values)}, {repr(self.default)})"
        )

    def __call__(self, attr: WidlExtAttribute):
        return single_arg_attribute(self.name, self.values, attr)


class FlagArg:
    def __init__(self, name: str):
        self.name = name
        self.default = False

    def __repr__(self):
        return f"FlagArg('{self.name}')"

    def __call__(self, attr: WidlExtAttribute):
        return flag_attribute(self.name, attr)


class StringArg:
    def __init__(self, name: str):
        self.name = name
        self.default = None

    def __repr__(self):
        return f"StringArg('{self.name}')"

    def __call__(self, attr: WidlExtAttribute):
        if len(attr.args) < 1:
            error(
                attr.name.pos, "`{}' requires at least one argument".format(self.name)
            )
        if len(attr.args) == 1:
            return attr.args[0].s
        return [arg.s for arg in attr.args]


class Guard:
    def __init__(self):
        self.name = "guard"
        self.default = None

    def __repr__(self):
        return f"Guard()"

    def __call__(self, attr: WidlExtAttribute):
        if len(attr.args) != 1:
            error(attr.name.pos, "`{}' requires exactly 1 argument".format(self.name))
        return attr.args[0].s if attr.args[0].s is not None else attr.args[0].code


class Guards:
    def __init__(self):
        self.name = "guards"
        self.default = []

    def __repr__(self):
        return f"Guards()"

    def __call__(self, attr: WidlExtAttribute):
        if len(attr.args) < 0:
            error(attr.name.pos, "`{}' requires at least 1 argument".format(self.name))
        return [arg.s if arg.s is not None else arg.code for arg in attr.args]


class Default:
    def __init__(self):
        self.name = "default"
        self.default = ""

    def __repr__(self):
        return f"Default()"

    def __call__(self, attr: WidlExtAttribute):
        if len(attr.args) != 1:
            error(attr.name.pos, "`{}' requires exactly 1 argument".format(self.name))
        return attr.args[0].s if attr.args[0].s is not None else attr.args[0].code


def _ext_attrs(ext_attrs: list[WidlExtAttribute], fields):
    result = {field.name: field.default for field in fields}
    fields = {field.name: field for field in fields}
    for attr in ext_attrs:
        code = attr.name.value.code
        try:
            field = fields[code]
        except KeyError:
            print(
                attr.name.pos,
                f"warning: unknown attribute `{attr.name.value.code}'",
                file=sys.stderr,
            )
            continue
        result[code] = field(attr)

    return result


_enum_ext_attrs = []


def enum_ext_attrs(ext_attrs: list[WidlExtAttribute]):
    return _ext_attrs(ext_attrs, _enum_ext_attrs)


_interface_ext_attrs = []


def interface_ext_attrs(ext_attrs: list[WidlExtAttribute]):
    return _ext_attrs(ext_attrs, _interface_ext_attrs)


_attribute_ext_attrs = [
    StringArg("if"),
    StringArg("var"),
    Default(),
    Guard(),
    Guards(),
]


def attribute_ext_attrs(ext_attrs: list[WidlExtAttribute]):
    return _ext_attrs(ext_attrs, _attribute_ext_attrs)


_oparg_ext_attrs = [
    FlagArg("defaulted"),
    FlagArg("in"),
    FlagArg("out"),
    Default(),
]


def oparg_ext_attrs(ext_attrs: list[WidlExtAttribute]):
    return _ext_attrs(ext_attrs, _oparg_ext_attrs)


_operation_ext_attrs = [
    FlagArg("in"),
    FlagArg("out"),
    FlagArg("unique"),
    FlagArg("span"),
    FlagArg("mutable"),
    FlagArg("throws"),
    FlagArg("static"),
    Guard(),
    Guards(),
]


def operation_ext_attrs(ext_attrs: list[WidlExtAttribute]):
    return _ext_attrs(ext_attrs, _operation_ext_attrs)


class ExtAttrDom(Enum):
    ENUM = "enum"
    INTERFACE = "interface"
    ATTRIBUTE = "attribute"
    OPERATION = "operation"
    ARGUMENT = "argument"


class ExtAttrType(Enum):
    BOOL = "bool"
    STR = "str"


def attr_list_from_domain(domain: ExtAttrDom):
    if domain == ExtAttrDom.ENUM.value:
        return _enum_ext_attrs
    if domain == ExtAttrDom.INTERFACE.value:
        return _interface_ext_attrs
    if domain == ExtAttrDom.ATTRIBUTE.value:
        return _attribute_ext_attrs
    if domain == ExtAttrDom.OPERATION.value:
        return _operation_ext_attrs
    if domain == ExtAttrDom.ARGUMENT.value:
        return _oparg_ext_attrs
    return []


def install_user_attrs(domain: ExtAttrDom, ext_attrs: dict[str, ExtAttrType]):
    dst = attr_list_from_domain(domain)
    for name in ext_attrs:
        kind = ext_attrs[name]
        if kind == ExtAttrType.BOOL.value:
            dst.append(FlagArg(name))
        elif kind == ExtAttrType.STR.value:
            dst.append(StringArg(name))
