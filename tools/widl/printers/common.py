# Copyright (c) 2025 Marcin Zdun
# This file is licensed under MIT license (see LICENSE for details)

TypeWithSource = tuple[str | None, str]
LanguageSpecificTypeSource = dict[str, TypeWithSource]


def make_simple(**langs: str | TypeWithSource) -> LanguageSpecificTypeSource:
    return {
        lang: (None, type_src) if isinstance(type_src, str) else type_src
        for lang, type_src in langs.items()
    }


simple_types: dict[str, LanguageSpecificTypeSource] = {
    "string": make_simple(cxx=("<string>", "std::string"), py3="str"),
    "string_view": make_simple(cxx=("<string_view>", "std::string_view"), py3="str"),
    "path": make_simple(
        cxx=("<filesystem>", "std::filesystem::path"), py3=("pathlib", "pathlib.Path")
    ),
    "int8_t": make_simple(cxx=("<cstdint>", "std::int8_t"), py3="int"),
    "int16_t": make_simple(cxx=("<cstdint>", "std::int16_t"), py3="int"),
    "int32_t": make_simple(cxx=("<cstdint>", "std::int32_t"), py3="int"),
    "int64_t": make_simple(cxx=("<cstdint>", "std::int64_t"), py3="int"),
    "uint8_t": make_simple(cxx=("<cstdint>", "std::uint8_t"), py3="int"),
    "uint16_t": make_simple(cxx=("<cstdint>", "std::uint16_t"), py3="int"),
    "uint32_t": make_simple(cxx=("<cstdint>", "std::uint32_t"), py3="int"),
    "uint64_t": make_simple(cxx=("<cstdint>", "std::uint64_t"), py3="int"),
    "void": make_simple(cxx="void", py3="None"),
    "int": make_simple(cxx="int", py3="int"),
    "unsigned": make_simple(cxx="unsigned", py3="int"),
    "short": make_simple(cxx="short", py3="int"),
    "long long": make_simple(cxx="long long", py3="int"),
    "size_t": make_simple(cxx="size_t", py3="int"),
    "char": make_simple(cxx="char", py3="str"),
    "bool": make_simple(cxx="bool", py3="bool"),
    "float": make_simple(cxx="float", py3="float"),
    "tuple": make_simple(cxx="tuple", py3="tuple"),
}

generic_types: dict[str, LanguageSpecificTypeSource] = {}
