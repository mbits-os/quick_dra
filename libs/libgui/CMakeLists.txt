# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(GUI_DEBUG_ENABLED_DEFAULT_VALUE ON)
else()
    set(GUI_DEBUG_ENABLED_DEFAULT_VALUE OFF)
endif()

if(UNIX)
    set(GUI_DEBUG_FRAMELESS_ENABLED OFF)
else()
    set(GUI_DEBUG_FRAMELESS_ENABLED ON)
endif()

option(GUI_DEBUG_ENABLED "Should --debug:<name> flags be available" ${GUI_DEBUG_ENABLED_DEFAULT_VALUE})

message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
message(STATUS "UNIX=${UNIX}")
message(STATUS "GUI_DEBUG_ENABLED=${GUI_DEBUG_ENABLED}")
message(STATUS "GUI_DEBUG_FRAMELESS_ENABLED=${GUI_DEBUG_FRAMELESS_ENABLED}")

set(SRCS
    include/quick_dra/gui/fs/common.hpp
    include/quick_dra/gui/fs/dir.hpp
    include/quick_dra/gui/fs/virtual.hpp
    include/quick_dra/gui/options/dbg_types.hpp
    include/quick_dra/gui/options/debug.hpp
    include/quick_dra/gui/options/options.hpp
    include/quick_dra/gui/webui.hpp
    include/tangle/decode.hpp
    include/tangle/uri.hpp
    src/fs/common.cpp
    src/fs/dir.cpp
    src/fs/virtual.cpp
    src/gui_tool.cpp
    src/options/debug.cpp
    src/options/options.cpp
    src/tangle/decode.cpp
    src/tangle/uri.cpp
    src/tarball.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})

add_library(libgui STATIC ${SRCS})

set(DEFS ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})

if(GUI_DEBUG_ENABLED)
    list(APPEND DEFS "-DGUI_DEBUG_ENABLED=1")
endif()

if(GUI_DEBUG_FRAMELESS_ENABLED)
    list(APPEND DEFS "-DGUI_DEBUG_FRAMELESS_ENABLED=1")
endif()

target_compile_options(libgui PRIVATE ${DEFS})
target_link_options(libgui PUBLIC ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_link_libraries(libgui PUBLIC libconv webui)
target_include_directories(libgui
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(libgui PROPERTIES FOLDER lib)

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
    add_subdirectory(tests/mock_webui)

    add_library(libgui_tested STATIC ${SRCS})

    target_compile_options(libgui_tested PRIVATE ${DEFS})
    target_link_options(libgui_tested PUBLIC ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
    target_include_directories(libgui_tested
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    target_link_libraries(libgui_tested PUBLIC libconv mock_webui)
    set_target_properties(libgui_tested PROPERTIES FOLDER tests/utils)

    set(FULL_TEST_SRCS
        tests/vfs.test.cpp
        tests/dirfs.test.cpp
        tests/options.test.cpp
    )
    set(MOCK_WEBUI_TEST_SRCS
        tests/gui_tool.test.cpp
    )
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/tests FILES
        ${FULL_TEST_SRCS}
        ${MOCK_WEBUI_TEST_SRCS}
    )

    add_project_test(libgui ${FULL_TEST_SRCS})
    target_link_libraries(libgui-test PUBLIC GTest::gmock_main libgui)

    add_project_test(libgui-mock ${MOCK_WEBUI_TEST_SRCS})
    target_link_libraries(libgui-mock-test PUBLIC GTest::gmock_main libgui_tested)
endif()
