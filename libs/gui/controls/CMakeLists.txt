# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

qt_standard_project_setup()

set(SRCS
    include/app/controls/Forms.hpp
    include/app/controls/forms/base.hpp
    include/app/controls/forms/ComboBox.hpp
    include/app/controls/forms/DocumentComboBox.hpp
    include/app/controls/forms/field.hpp
    include/app/controls/forms/LineEdit.hpp
    include/app/controls/forms/TableView.hpp
    include/app/controls/Glyph.hpp
    include/app/controls/PageScrollArea.hpp
    include/app/controls/Panel_p.hpp
    include/app/controls/Panel.hpp
    include/app/controls/PanelButton_p.hpp
    include/app/controls/PanelButton.hpp
    include/app/controls/PanelButtonGroup_p.hpp
    include/app/controls/PanelButtonGroup.hpp
    include/app/controls/PanelButtonStyle.hpp
    include/app/controls/WordWrap.hpp
    src/app/controls/forms/ComboBox.cpp
    src/app/controls/forms/DocumentComboBox.cpp
    src/app/controls/forms/LineEdit.cpp
    src/app/controls/forms/TableView.cpp
    src/app/controls/Glyph.cpp
    src/app/controls/PageScrollArea.cpp
    src/app/controls/Panel.cpp
    src/app/controls/PanelButton.cpp
    src/app/controls/PanelButtonGroup.cpp
    src/app/controls/WordWrap.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})

qt_add_library(controls STATIC ${SRCS})

target_compile_options(controls PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
target_link_options(controls PUBLIC ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_link_libraries(controls PUBLIC gui)
target_include_directories(controls
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(controls PROPERTIES FOLDER lib)

# #################################################################
# #  INSTALL
# #################################################################
if(QUICK_DRA_INSTALL)
    install(TARGETS controls COMPONENT gui_libraries)

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
    set(FULL_TEST_SRCS
        tests/ControlsTest.cpp
        tests/ControlsTest.hpp
        tests/Forms.test.cpp
        tests/Glyph.test.cpp
        tests/PageScrollArea.test.cpp
        tests/PanelButtonGroup.test.cpp
        tests/WordWrap.test.cpp
    )

    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/tests FILES
        ${FULL_TEST_SRCS}
    )

    execute_process(COMMAND ${Python3_EXECUTABLE}
        "${PROJECT_SOURCE_DIR}/.flow/flow.py" system --format=os
        OUTPUT_VARIABLE OS_NAME
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    string(REGEX REPLACE "^(.*)-latest$" "\\1" OS_NAME "${OS_NAME}")
    string(REGEX REPLACE "\\.[0-9]+$" "" OS_NAME "${OS_NAME}")
    string(REGEX REPLACE "-[0-9]+$" "" OS_NAME_PREFIX "${OS_NAME}")

    # Ubuntu 24.04 => "ubuntu-14"
    # Ubuntu 26.04 => "ubuntu"
    # Ubuntu 28.04 => "ubuntu" => changes in font renderer shown
    if(OS_NAME_PREFIX STREQUAL "ubuntu" AND NOT(OS_NAME STREQUAL "ubuntu-24"))
        set(OS_NAME "ubuntu")
    endif()

    qt_add_project_test(controls ${FULL_TEST_SRCS})
    target_link_libraries(controls-test PUBLIC testing controls)
    target_compile_definitions(controls-test PUBLIC "OS_NAME=\"${OS_NAME}\"")
endif()
