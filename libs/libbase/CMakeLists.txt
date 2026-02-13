# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

configure_file(src/version.hpp.in include/quick_dra/version.hpp @ONLY)
configure_file(src/base/dir_names.hpp.in include/quick_dra/base/dir_names.hpp @ONLY)

set(GEN_SRCS
    "${CMAKE_CURRENT_BINARY_DIR}/include/quick_dra/version.hpp"
    "${CMAKE_CURRENT_BINARY_DIR}/include/quick_dra/base/dir_names.hpp"
)

set(SRCS
    include/quick_dra/base/meta.hpp
    include/quick_dra/base/paths.hpp
    include/quick_dra/base/str.hpp
    include/quick_dra/base/types.hpp
    include/quick_dra/base/verbose.hpp
    src/base/paths.cpp
    src/base/str.cpp
    src/base/types.cpp
)

if(UNIX)
    list(APPEND SRCS src/base/exec_path_posix.cpp)
elseif(WIN32)
    list(APPEND SRCS src/base/exec_path_win32.cpp)
endif()

if(TARGET ICU::i18n)
    list(APPEND SRCS src/base/unicode_icu.cpp)
elseif(WIN32)
    list(APPEND SRCS src/base/unicode_win32.cpp)
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})
source_group(TREE ${CMAKE_CURRENT_BINARY_DIR} PREFIX gen FILES ${GEN_SRCS})

add_library(libbase STATIC ${GEN_SRCS} ${SRCS})

target_compile_options(libbase PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
target_link_options(libbase PUBLIC ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_include_directories(libbase
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/src
)
target_link_libraries(libbase PUBLIC fmt::fmt)

if(TARGET ICU::i18n)
    message(STATUS "Component target declared 'ICU::i18n'")
    target_link_libraries(libbase PUBLIC ICU::uc ICU::i18n ${ICU_LIBRARIES})
endif()

# #################################################################
# #  INSTALL
# #################################################################
if(QUICK_DRA_INSTALL)
    install(TARGETS libbase COMPONENT libraries)

    install(
        DIRECTORY
        include/
        ${CMAKE_CURRENT_BINARY_DIR}/include/
        TYPE INCLUDE
        COMPONENT libraries
    )

    cpack_add_component(libraries
        DISPLAY_NAME "Quick-DRA Libraries"
        DESCRIPTION "Library and include files"
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

    add_project_test(libbase ${EXCLUDES_TEST_SRCS_CC} ${EXCLUDES_TEST_SRCS_CPP} ${EXCLUDES_TEST_SRCS_CXX})
    target_link_libraries(libbase-test PUBLIC GTest::gmock_main libbase)

    add_test(NAME libbase COMMAND libbase-test "--gtest_output=xml:${TEST_REPORT_DIR}/libbase/${TEST_REPORT_FILE}")
endif()
