# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

set(SRCS
    include/quick_dra/gui/vfs.hpp
    src/tarball.cpp
    src/tool.cpp
    src/vfs.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})

add_library(libgui STATIC ${SRCS})

target_compile_options(libgui PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
target_link_options(libgui PUBLIC ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_link_libraries(libgui libconv webui)
target_include_directories(libgui
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# #################################################################
# #  INSTALL
# #################################################################
if(QUICK_DRA_INSTALL)
    install(TARGETS libgui COMPONENT libraries)

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
    file(GLOB CLI_TEST_SRCS_CC tests/*.cc)
    file(GLOB CLI_TEST_SRCS_CPP tests/*.cpp)
    file(GLOB CLI_TEST_SRCS_CXX tests/*.cxx)
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/tests FILES
        ${CLI_TEST_SRCS_CC}
        ${CLI_TEST_SRCS_CPP}
        ${CLI_TEST_SRCS_CXX})

    add_project_test(libgui ${CLI_TEST_SRCS_CC} ${CLI_TEST_SRCS_CPP} ${CLI_TEST_SRCS_CXX})
    target_link_libraries(libgui-test PUBLIC GTest::gmock_main libgui)

    set(_OUTPUT ${PROJECT_BINARY_DIR}/bin)

    set_target_properties(libgui-test PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${_OUTPUT}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${_OUTPUT}
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${_OUTPUT}
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${_OUTPUT}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${_OUTPUT}
    )
endif()
