simple_types: dict[str, tuple[str | None, str]] = {
    "string": ("<string>", "std::string"),
    "string_view": ("<string_view>", "std::string_view"),
    "path": ("<filesystem>", "std::filesystem::path"),
    "int8_t": ("<cstdint>", "std::int8_t"),
    "int16_t": ("<cstdint>", "std::int16_t"),
    "int32_t": ("<cstdint>", "std::int32_t"),
    "int64_t": ("<cstdint>", "std::int64_t"),
    "uint8_t": ("<cstdint>", "std::uint8_t"),
    "uint16_t": ("<cstdint>", "std::uint16_t"),
    "uint32_t": ("<cstdint>", "std::uint32_t"),
    "uint64_t": ("<cstdint>", "std::uint64_t"),
}

generic_types: dict[str, tuple[str | None, str]] = {}

builtin_types = [
    "void",
    "int",
    "unsigned",
    "short",
    "long long",
    "size_t",
    "char",
    "bool",
    "float",
    "tuple",
]
