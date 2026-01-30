set(GRAPHVIZ_GRAPH_HEADER "graph [\n  fontname = \"Montserrat SemiBold\";\n];\nnode [\n  fontsize = \"12\";\n  fontname = \"Montserrat\";\n];\nedge [\n  fontname = \"Montserrat Light\";\n];")
set(GRAPHVIZ_GENERATE_PER_TARGET FALSE)
set(GRAPHVIZ_GENERATE_DEPENDERS FALSE)
set(GRAPHVIZ_IGNORE_TARGETS
    "GTest::.*"
    ".*-test"
    # Windows:
    advapi32
    bcrypt
    crypt32
    iphlpapi
    user32
    ws2_32
    # Conan internals
    "CONAN_LIB::.*"
    ".*_DEPS_TARGET"
    # Conan "disjonts"
    "c4core::c4core"
    "FastFloat::fast_float"
    "ZLIB::ZLIB"
    "openssl::.*"
    "OpenSSL::.*"
)
