# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

set(SRCS
    include/quick_dra/lex/tax.hpp
    include/quick_dra/lex/validators.hpp
    src/lex/tax.cpp
    src/lex/validate/actions.hpp
    src/lex/validate/check_compiler.hpp
    src/lex/validate/fixed_string.hpp
    src/lex/validators.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})
add_library(liblex STATIC ${SRCS})

target_compile_options(liblex PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
target_link_options(liblex PUBLIC ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_include_directories(liblex
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/src
)
target_link_libraries(liblex PUBLIC libbase)

# #################################################################
# #  INSTALL
# #################################################################
if(QUICK_DRA_INSTALL)
    install(TARGETS liblex COMPONENT libraries)

    install(
        DIRECTORY include/
        TYPE INCLUDE
        COMPONENT libraries
    )
endif()

# #################################################################
# #  TESTING
# #################################################################
if(QUICK_DRA_TESTING)
    file(GLOB LEX_TEST_SRCS_CC tests/*.cc)
    file(GLOB LEX_TEST_SRCS_CPP tests/*.cpp)
    file(GLOB LEX_TEST_SRCS_CXX tests/*.cxx)
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/tests FILES
        ${LEX_TEST_SRCS_CC}
        ${LEX_TEST_SRCS_CPP}
        ${LEX_TEST_SRCS_CXX})

    add_project_test(liblex ${LEX_TEST_SRCS_CC} ${LEX_TEST_SRCS_CPP} ${LEX_TEST_SRCS_CXX})
    target_link_libraries(liblex-test PUBLIC GTest::gmock_main liblex)

    add_test(NAME liblex COMMAND liblex-test "--gtest_output=xml:${TEST_REPORT_DIR}/liblex/${TEST_REPORT_FILE}")
endif()
