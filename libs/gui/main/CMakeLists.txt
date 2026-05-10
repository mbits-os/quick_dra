# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

qt_standard_project_setup()

set(SRCS
    include/app/main/MainWindow.hpp
    include/app/main/options.hpp
    src/app/main/gui_tool.cpp
    src/app/main/MainWindow.cpp
    src/app/main/MainWindowUi.cpp
    src/app/main/options.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})

qt_add_library(main STATIC ${SRCS})

target_compile_options(main PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
target_link_options(main PUBLIC ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_link_libraries(main PUBLIC pages)
target_include_directories(main
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(main PROPERTIES FOLDER lib)

# #################################################################
# #  INSTALL
# #################################################################
if(QUICK_DRA_INSTALL)
    install(TARGETS main COMPONENT libraries)

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
    set(FULL_TEST_SRCS
        tests/options.test.cpp
    )
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/tests FILES
        ${FULL_TEST_SRCS}
    )

    add_project_test(main ${FULL_TEST_SRCS})
    target_link_libraries(main-test PUBLIC GTest::gmock_main main)
endif()
