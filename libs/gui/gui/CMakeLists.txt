# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

qt_standard_project_setup()

set(SRCS
    include/app/gui/CurrentColor.hpp
    include/app/gui/Globals.hpp
    include/app/gui/PagedWidget.hpp
    include/app/gui/PageHeader.hpp
    include/app/gui/PageStack.hpp
    src/app/gui/CurrentColor.cpp
    src/app/gui/Globals.cpp
    src/app/gui/PagedWidget.cpp
    src/app/gui/PageHeader.cpp
    src/app/gui/PageStack.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})

qt_add_library(gui STATIC ${SRCS})

target_compile_options(gui PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
target_link_options(gui PUBLIC ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_link_libraries(gui PUBLIC utils)
target_include_directories(gui
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(gui PROPERTIES FOLDER lib)

# #################################################################
# #  INSTALL
# #################################################################
if(QUICK_DRA_INSTALL)
    install(TARGETS gui COMPONENT libraries)

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

    add_project_test(gui ${FULL_TEST_SRCS})
    target_link_libraries(gui-test PUBLIC GTest::gmock_main gui)
endif()
