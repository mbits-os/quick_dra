# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

qt_standard_project_setup()

set(SRCS
    include/app/utils/FormData.hpp
    include/app/utils/forms.hpp
    include/app/utils/LaidOut.hpp
    include/app/utils/str.hpp
    include/app/utils/utils.hpp
    src/app/utils/FormData.cpp
    src/app/utils/forms.cpp
    src/app/utils/utils.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})

qt_add_library(utils STATIC ${SRCS})

target_compile_options(utils PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
target_link_options(utils PUBLIC ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_link_libraries(utils PUBLIC libconv Qt6::Gui Qt6::Widgets Qt6::Svg)
target_include_directories(utils
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(utils PROPERTIES FOLDER lib)

# #################################################################
# #  INSTALL
# #################################################################
if(QUICK_DRA_INSTALL)
    install(TARGETS utils COMPONENT gui_libraries)

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
    configure_file(tests/FormDataPaths.hpp.in tests/FormDataPaths.hpp @ONLY)

    set(FULL_TEST_GEN_SRCS
        "${CMAKE_CURRENT_BINARY_DIR}/tests/FormDataPaths.hpp"
    )

    set(FULL_TEST_SRCS
        tests/FormData.test.cpp
        tests/forms.test.cpp
        tests/LaidOut.test.cpp
        tests/utils.test.cpp
        tests/report_format.qrc
    )
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/tests FILES ${FULL_TEST_SRCS})
    source_group(TREE ${CMAKE_CURRENT_BINARY_DIR} PREFIX gen FILES ${FULL_TEST_GEN_SRCS})

    add_project_test(utils ${FULL_TEST_SRCS} ${FULL_TEST_GEN_SRCS})
    target_link_libraries(utils-test PUBLIC GTest::gmock_main utils)
    target_include_directories(utils-test
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
        ${CMAKE_CURRENT_BINARY_DIR}/tests
    )
    set_target_properties(utils-test PROPERTIES AUTORCC ON)
endif()
