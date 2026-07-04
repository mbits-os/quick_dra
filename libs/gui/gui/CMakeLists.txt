# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

qt_standard_project_setup()

set(SRCS
    include/app/gui/CurrentColor.hpp
    include/app/gui/Globals.hpp
    include/app/gui/HeaderTitle_p.hpp
    include/app/gui/HeaderTitle.hpp
    include/app/gui/PagedWidget.hpp
    include/app/gui/PageHeader.hpp
    include/app/gui/PageStack.hpp
    include/app/gui/types.hpp
    src/app/gui/CurrentColor.cpp
    src/app/gui/Globals.cpp
    src/app/gui/HeaderTitle.cpp
    src/app/gui/PagedWidget.cpp
    src/app/gui/PageHeader.cpp
    src/app/gui/PageStack.cpp
)

set(RES res/icons.qrc)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS} ${RES})

qt_add_library(gui STATIC ${SRCS})

target_sources(gui INTERFACE ${RES})
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
    install(TARGETS gui COMPONENT gui_libraries)

    install(
        DIRECTORY include/
        TYPE INCLUDE
        COMPONENT gui_libraries
    )
endif()

# #################################################################
# #  TESTING
# #################################################################
if(QUICK_DRA_TESTING)
    get_target_property(QDRA_SOURCE_DIR qdra SOURCE_DIR)
    configure_file(tests/EnvPaths.hpp.in EnvPaths.hpp @ONLY)

    set(FULL_TEST_GEN_SRCS
        "${CMAKE_CURRENT_BINARY_DIR}/EnvPaths.hpp"
    )

    set(FULL_TEST_SRCS
        tests/Pages.test.cpp
        tests/Globals.test.cpp
    )

    set(QT6_TEST_SRCS
        tests/qt6/GuiTest.cpp
        tests/qt6/GuiTest.hpp
        tests/qt6/CurrentColor.test.cpp
        tests/qt6/PageHeader.test.cpp
    )

    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/tests FILES ${FULL_TEST_SRCS} ${QT6_TEST_SRCS})
    source_group(TREE ${CMAKE_CURRENT_BINARY_DIR} PREFIX gen FILES ${FULL_TEST_GEN_SRCS})

    add_project_test(gui ${FULL_TEST_SRCS} ${FULL_TEST_GEN_SRCS})
    target_link_libraries(gui-test PUBLIC GTest::gmock_main testing gui)

    qt_add_project_test(gui-qt6 ${QT6_TEST_SRCS})
    target_link_libraries(gui-qt6-test PUBLIC testing gui)
endif()
