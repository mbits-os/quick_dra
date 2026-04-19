# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

set(SRCS
    include/quick_dra/gui/dirfs.hpp
    include/quick_dra/gui/fs_common.hpp
    include/quick_dra/gui/vfs.hpp
    include/quick_dra/gui/webui.hpp
    src/dirfs.cpp
    src/fs_common.cpp
    src/tarball.cpp
    src/tool.cpp
    src/vfs.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})

add_library(libgui STATIC ${SRCS})

target_compile_options(libgui PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
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

    target_compile_options(libgui_tested PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
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
