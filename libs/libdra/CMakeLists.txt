# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

configure_file(src/version.hpp.in include/quick_dra/version.hpp @ONLY)
configure_file(src/base/dir_names.hpp.in include/quick_dra/base/dir_names.hpp @ONLY)

set(GEN_SRCS
    "${CMAKE_CURRENT_BINARY_DIR}/include/quick_dra/version.hpp"
    "${CMAKE_CURRENT_BINARY_DIR}/include/quick_dra/base/dir_names.hpp"
    "${CMAKE_CURRENT_BINARY_DIR}/include/quick_dra/model/yaml.hpp"
    "${CMAKE_CURRENT_BINARY_DIR}/src/model/yaml_parser.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/src/model/yaml_parser.hpp"
)

set(SRCS
    include/quick_dra/base/meta.hpp
    include/quick_dra/base/paths.hpp
    include/quick_dra/base/str.hpp
    include/quick_dra/base/verbose.hpp
    include/quick_dra/docs/file_set.hpp
    include/quick_dra/docs/forms.hpp
    include/quick_dra/docs/xml_builder.hpp
    include/quick_dra/docs/xml.hpp
    include/quick_dra/io/http.hpp
    include/quick_dra/io/options.hpp
    include/quick_dra/model/base_types.hpp
    include/quick_dra/model/model.hpp
    include/quick_dra/model/types.hpp
    src/base/paths.cpp
    src/base/str.cpp
    src/docs/file_set.cpp
    src/docs/forms.cpp
    src/docs/xml_builder.cpp
    src/docs/xml.cpp
    src/io/http.cpp
    src/io/options.cpp
    src/model/_parser_types.hpp
    src/model/base_types.cpp
    src/model/compiler.cpp
    src/model/idl/config.json
    src/model/idl/quick_dra.webidl
    src/model/parser_debug.cpp
    src/model/parser_impl.cpp
)

if(UNIX)
    list(APPEND SRCS src/base/posix.cpp)
elseif(WIN32)
    list(APPEND SRCS src/base/win32.cpp)
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})
source_group(TREE ${CMAKE_CURRENT_BINARY_DIR} PREFIX gen FILES ${GEN_SRCS})

add_custom_command(
    OUTPUT
    "${CMAKE_CURRENT_BINARY_DIR}/include/quick_dra/model/yaml.hpp"
    "${CMAKE_CURRENT_BINARY_DIR}/src/model/yaml_parser.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/src/model/yaml_parser.hpp"
    COMMAND
    Python3::Interpreter
    "${PROJECT_SOURCE_DIR}/tools/webidl.py"
    -d "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
    -d "CMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    -d "PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}"
    -d "PROJECT_BINARY_DIR=${PROJECT_BINARY_DIR}"
    -c "${CMAKE_CURRENT_SOURCE_DIR}/src/model/idl/config.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/model/idl/quick_dra.webidl"
    DEPENDS
    "${PROJECT_SOURCE_DIR}/tools/webidl.py"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/model/idl/quick_dra.webidl"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/model/idl/config.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/model/idl/cpp/code.mustache"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/model/idl/cpp/header.mustache"
    "${PROJECT_SOURCE_DIR}/tools/widl/printers/cpp/common.py"
    VERBATIM)

add_library(libdra STATIC ${GEN_SRCS} ${SRCS})

target_compile_options(libdra PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
target_link_options(libdra PUBLIC ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_include_directories(libdra
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/src
)
target_link_libraries(libdra PUBLIC fmt::fmt ryml::ryml CURL::libcurl)

add_custom_command(
    TARGET libdra POST_BUILD
    COMMENT "Copy config files"
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy_directory "${PROJECT_SOURCE_DIR}/data/config"
    "${PROJECT_BINARY_DIR}/${SHARE_DIR}/config")

# #################################################################
# #  INSTALL
# #################################################################
if(QUICK_DRA_INSTALL)
    install(TARGETS libdra COMPONENT libdra)

    install(
        DIRECTORY
        include/
        ${CMAKE_CURRENT_BINARY_DIR}/include/
        TYPE INCLUDE
        COMPONENT libdra
    )

    cpack_add_component(libdra
        DISPLAY_NAME "Quick-DRA Library"
        DESCRIPTION "Main library and include files"
        DISABLED
        GROUP devel
    )
endif()

# #################################################################
# #  TESTING
# #################################################################
if(QUICK_DRA_TESTING)
    file(GLOB EXCLUDES_TEST_SRCS_CC tests/*.cc)
    file(GLOB EXCLUDES_TEST_SRCS_CPP tests/*.cpp)
    file(GLOB EXCLUDES_TEST_SRCS_CXX tests/*.cxx)
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/tests FILES
        ${EXCLUDES_TEST_SRCS_CC}
        ${EXCLUDES_TEST_SRCS_CPP}
        ${EXCLUDES_TEST_SRCS_CXX})

    add_project_test(libdra ${EXCLUDES_TEST_SRCS_CC} ${EXCLUDES_TEST_SRCS_CPP} ${EXCLUDES_TEST_SRCS_CXX})
    target_link_libraries(libdra-test PUBLIC GTest::gmock_main fmt::fmt libdra)

    add_test(NAME libdra COMMAND libdra-test "--gtest_output=xml:${TEST_REPORT_DIR}/libdra/${TEST_REPORT_FILE}")
endif()
