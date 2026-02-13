# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

cmake_minimum_required(VERSION 3.28)
project(quick_dra
  VERSION 0.9.0
  DESCRIPTION "Generate simple ZUS RCA/DRA document set."
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
set(QUICK_DRA_W_ERROR OFF CACHE BOOL "Compile with warnings turned to errors")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_BINARY_DIR}/conan" "${PROJECT_SOURCE_DIR}/.flow/cmake")
list(APPEND CMAKE_PREFIX_PATH "${PROJECT_BINARY_DIR}/conan")

include(coverage)
include(common)
include(output_dirs_setup)
prepare_output_dirs_setup()

set(CONAN_CMAKE_SILENT_OUTPUT ON)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
find_package(fmt REQUIRED)
find_package(mbits-args REQUIRED)
find_package(ryml REQUIRED)
find_package(CURL REQUIRED)
find_package(ctre REQUIRED)

if(UNIX)
  find_package(ICU COMPONENTS uc i18n REQUIRED)
endif()

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

cpack_add_component_group(apps
  DISPLAY_NAME Applications
  DESCRIPTION "End-user applications"
  EXPANDED
)

cpack_add_component_group(devel
  DISPLAY_NAME "Development"
  DESCRIPTION "Development tools and libraries"
  EXPANDED
)

if(QUICK_DRA_TESTING)
  set(COVERALLS_PREFIX QUICK_DRA_)
  include(${PROJECT_SOURCE_DIR}/tools/coveralls/build_flags.cmake)
endif()

add_subdirectory(libs)

# #################################################################
# #  COVERAGE
# #################################################################
execute_process(COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/.flow/flow.py"
  system --format props
  OUTPUT_VARIABLE QUICK_DRA_COV_PROPERTIES
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/.flow/flow.py"
  system --format os
  OUTPUT_VARIABLE QUICK_DRA_COV_OS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(TOLOWER ${CMAKE_CXX_COMPILER_ID} QUICK_DRA_COV_PROPERTIES_COMPILER)

if(QUICK_DRA_COV_PROPERTIES_COMPILER STREQUAL "gnu")
  set(QUICK_DRA_COV_PROPERTIES_COMPILER "gcc")
endif()

string(APPEND QUICK_DRA_COV_PROPERTIES "\n-pbuild_type='${CMAKE_BUILD_TYPE}'")
string(APPEND QUICK_DRA_COV_PROPERTIES "\n-pcompiler='${QUICK_DRA_COV_PROPERTIES_COMPILER}'")

if(MSVC)
  string(APPEND QUICK_DRA_COV_PROPERTIES "\n-pcompiler.version='${MSVC_VERSION}'")
  string(APPEND QUICK_DRA_COV_PROPERTIES "\n-pcompiler.toolset='${CMAKE_VS_PLATFORM_TOOLSET}'")
else()
  string(APPEND QUICK_DRA_COV_PROPERTIES "\n-pcompiler.version='${CMAKE_CXX_COMPILER_VERSION}'")
endif()

if("stdc++" IN_LIST CMAKE_CXX_IMPLICIT_LINK_LIBRARIES)
  string(APPEND QUICK_DRA_COV_PROPERTIES "\n-pcompiler.libcxx='stdc++'")
elseif("c++" IN_LIST CMAKE_CXX_IMPLICIT_LINK_LIBRARIES)
  string(APPEND QUICK_DRA_COV_PROPERTIES "\n-pcompiler.libcxx='c++'")
endif()

if(QUICK_DRA_SANITIZE)
  string(APPEND QUICK_DRA_COV_PROPERTIES "\n-psanitizer=on")
else()
  string(APPEND QUICK_DRA_COV_PROPERTIES "\n-psanitizer=off")
endif()

if(QUICK_DRA_TESTING)
  set(COVERALLS_PREFIX QUICK_DRA_)

  set(QUICK_DRA_COVERALLS_DIRS ${LEGACY_COVERAGE})
  set(QUICK_DRA_COVERALLS_TARGET qdra)

  # set(__SANITIZER)

  # if(QUICK_DRA_SANITIZE)
  # set(__SANITIZER -sanitizer)
  # endif()

  # set(QUICK_DRA_COVERALLS_FILE "${PROJECT_BINARY_DIR}/../artifacts/coverage/coveralls_${QUICK_DRA_COV_OS}-${CMAKE_BUILD_TYPE}-${QUICK_DRA_COV_PROPERTIES_COMPILER}${__SANITIZER}.json")
  include(${PROJECT_SOURCE_DIR}/tools/coveralls/Coveralls.cmake)
endif()

file(GENERATE OUTPUT "${PROJECT_BINARY_DIR}/report_answers.txt" CONTENT "${QUICK_DRA_COV_PROPERTIES}\n")
string(REPLACE ";" "\n    include = " SRC_INCLUDE "${COVERED_CODE}")
string(REPLACE ";" "\n    exclude = " SRC_EXCLUDE "${NOT_COVERED_CODE}")
configure_file("${PROJECT_SOURCE_DIR}/.covcollect.in" "${PROJECT_BINARY_DIR}/.covcollect" @ONLY)
