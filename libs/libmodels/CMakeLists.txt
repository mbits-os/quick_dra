set(GEN_SRCS
    "${CMAKE_CURRENT_BINARY_DIR}/include/quick_dra/models/yaml.hpp"
    "${CMAKE_CURRENT_BINARY_DIR}/src/models/yaml_parser.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/src/models/yaml_parser.hpp"
)

set(SRCS
    include/quick_dra/models/base_types.hpp
    include/quick_dra/models/model.hpp
    include/quick_dra/models/types.hpp
    src/models/_parser_types.hpp
    src/models/base_types.cpp
    src/models/compiler.cpp
    src/models/parser_debug.cpp
    src/models/parser_impl.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})
source_group(TREE ${CMAKE_CURRENT_BINARY_DIR} PREFIX gen FILES ${GEN_SRCS})

add_custom_command(
    OUTPUT
    "${CMAKE_CURRENT_BINARY_DIR}/include/quick_dra/models/yaml.hpp"
    "${CMAKE_CURRENT_BINARY_DIR}/src/models/yaml_parser.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/src/models/yaml_parser.hpp"
    COMMAND
    Python3::Interpreter
    "${PROJECT_SOURCE_DIR}/tools/webidl.py"
    -d "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
    -d "CMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    -d "PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}"
    -d "PROJECT_BINARY_DIR=${PROJECT_BINARY_DIR}"
    -c "${CMAKE_CURRENT_SOURCE_DIR}/idl/config.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/idl/quick_dra.webidl"
    DEPENDS
    "${PROJECT_SOURCE_DIR}/tools/webidl.py"
    "${CMAKE_CURRENT_SOURCE_DIR}/idl/quick_dra.webidl"
    "${CMAKE_CURRENT_SOURCE_DIR}/idl/config.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/idl/cpp/code.mustache"
    "${CMAKE_CURRENT_SOURCE_DIR}/idl/cpp/header.mustache"
    "${PROJECT_SOURCE_DIR}/tools/widl/printers/cpp/common.py"
    VERBATIM)

add_library(libmodels STATIC ${SRCS} ${GEN_SRCS})

target_compile_options(libmodels PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
target_link_options(libmodels PUBLIC ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_include_directories(libmodels
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/src
)
target_link_libraries(libmodels PUBLIC ryml::ryml libbase)

# #################################################################
# #  INSTALL
# #################################################################
if(QUICK_DRA_INSTALL)
    install(TARGETS libmodels COMPONENT libraries)

    install(
        DIRECTORY
        include/
        ${CMAKE_CURRENT_BINARY_DIR}/include/
        TYPE INCLUDE
        COMPONENT libraries
    )
endif()
