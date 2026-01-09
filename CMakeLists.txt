# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

cmake_minimum_required(VERSION 3.28)
project(quick_dra
  VERSION 0.2.0
  DESCRIPTION ""
  LANGUAGES CXX)

set(PROJECT_VERSION_SHORT "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
set(PROJECT_VERSION_STABILITY "" # or "-alpha", or "-beta", or "-rc.1", or "-rc.2"
  CACHE STRING "" FORCE)

if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
  message(FATAL_ERROR "Building in source directory is not supported")
endif()

set(SHARE_DIR "share/${PROJECT_NAME}-${PROJECT_VERSION_SHORT}")

set(QUICK_DRA_TESTING ON CACHE BOOL "Compile and/or run self-tests")
set(QUICK_DRA_INSTALL ON CACHE BOOL "Install the application")
set(QUICK_DRA_SANITIZE OFF CACHE BOOL "Compile with sanitizers enabled")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_BINARY_DIR}/conan" "${PROJECT_SOURCE_DIR}/.flow/cmake")
list(APPEND CMAKE_PREFIX_PATH "${PROJECT_BINARY_DIR}/conan")

include(common)
include(output_dirs_setup)
old_conan_output_dirs_setup()

set(CONAN_CMAKE_SILENT_OUTPUT ON)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
find_package(fmt REQUIRED)
find_package(GTest REQUIRED)
find_package(mbits-args REQUIRED)
find_package(ryml REQUIRED)
find_package(CURL REQUIRED)

if(QUICK_DRA_TESTING)
  enable_testing()

  find_package(GTest REQUIRED)

  set(TEST_REPORT_DIR "${PROJECT_BINARY_DIR}/test-results")
  set(TEST_SANITIZER_TAG "")

  if(QUICK_DRA_SANITIZE)
    set(TEST_SANITIZER_TAG "-sanitizer")
  endif()

  set(TEST_REPORT_FILE "${CMAKE_SYSTEM_NAME}-${CMAKE_CXX_COMPILER_ID}-${CMAKE_BUILD_TYPE}${TEST_SANITIZER_TAG}.xml")
endif()

include(${PROJECT_SOURCE_DIR}/.flow/packages/config.cmake)
include(CPack)

configure_file(src/app/version.hpp.in include/quick_dra/app/version.hpp @ONLY)
configure_file(src/base/dir_names.hpp.in include/quick_dra/base/dir_names.hpp @ONLY)

set(SRCS
  "${CMAKE_CURRENT_BINARY_DIR}/include/quick_dra/app/version.hpp"
  "${CMAKE_CURRENT_BINARY_DIR}/include/quick_dra/base/dir_names.hpp"
  "${CMAKE_CURRENT_BINARY_DIR}/include/quick_dra/model/yaml.hpp"
  "${CMAKE_CURRENT_BINARY_DIR}/src/model/yaml_parser.cpp"
  "${CMAKE_CURRENT_BINARY_DIR}/src/model/yaml_parser.hpp"
  include/quick_dra/base/meta.hpp
  include/quick_dra/base/paths.hpp
  include/quick_dra/base/str.hpp
  include/quick_dra/base/verbose.hpp
  include/quick_dra/docs/file_set.hpp
  include/quick_dra/docs/forms.hpp
  include/quick_dra/docs/xml_builder.hpp
  include/quick_dra/docs/xml.hpp
  include/quick_dra/io/config.hpp
  include/quick_dra/io/http.hpp
  include/quick_dra/model/base_types.hpp
  include/quick_dra/model/model.hpp
  include/quick_dra/model/types.hpp
  src/app/main.cpp
  src/base/paths.cpp
  src/base/str.cpp
  src/docs/file_set.cpp
  src/docs/forms.cpp
  src/docs/xml_builder.cpp
  src/docs/xml.cpp
  src/io/config.cpp
  src/io/http.cpp
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

add_executable(${PROJECT_NAME} ${SRCS})

add_win32_icon(${PROJECT_NAME} "appicon.ico")
target_compile_options(${PROJECT_NAME} PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
target_link_options(${PROJECT_NAME} PUBLIC ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_include_directories(${PROJECT_NAME}
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}/include
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_BINARY_DIR}/src
)
target_link_libraries(${PROJECT_NAME} fmt::fmt ryml::ryml CURL::libcurl mbits::args)
set_target_properties(${PROJECT_NAME} PROPERTIES
  VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

foreach(FILE minimal_pay.yaml templates.yaml)
  add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${PROJECT_SOURCE_DIR}/data/config/${FILE}"
    "${CMAKE_CURRENT_BINARY_DIR}/${SHARE_DIR}/config/${FILE}")
endforeach(FILE)

# #################################################################
# #  INSTALL
# #################################################################
if(QUICK_DRA_INSTALL)
  install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${BINARY_DIR}
    COMPONENT app
  )

  install(
    DIRECTORY ${PROJECT_BINARY_DIR}/share/
    DESTINATION share/
    USE_SOURCE_PERMISSIONS
    COMPONENT app
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

  add_project_test(${PROJECT_NAME} ${EXCLUDES_TEST_SRCS_CC} ${EXCLUDES_TEST_SRCS_CPP} ${EXCLUDES_TEST_SRCS_CXX})
  target_link_libraries(${PROJECT_NAME}-test PUBLIC GTest::gmock_main fmt::fmt)

  add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME}-test "--gtest_output=xml:${TEST_REPORT_DIR}/${PROJECT_NAME}/${TEST_REPORT_FILE}")
endif()
