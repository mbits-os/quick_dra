# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

qt_standard_project_setup()

set(SRCS
    include/app/controls/Glyph.hpp
    include/app/controls/PageScrollArea.hpp
    include/app/controls/PanelButtonGroup_p.hpp
    include/app/controls/PanelButtonGroup.hpp
    src/app/controls/Glyph.cpp
    src/app/controls/PageScrollArea.cpp
    src/app/controls/PanelButtonGroup.cpp
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
    install(TARGETS controls COMPONENT libraries)

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

    add_project_test(controls ${FULL_TEST_SRCS})
    target_link_libraries(controls-test PUBLIC GTest::gmock_main controls)
endif()
