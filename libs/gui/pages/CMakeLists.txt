# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

qt_standard_project_setup()

set(SRCS
    include/app/pages/HomePage.hpp
    include/app/pages/InsuredEditPage.hpp
    include/app/pages/PayerEditPage.hpp
    include/app/pages/PersonelPage.hpp
    include/app/pages/RemoveHistoryPage.hpp
    include/app/pages/RemoveInsuredPage.hpp
    include/app/pages/ReportFormPage.hpp
    include/app/pages/ReportIdEditPage.hpp
    src/app/pages/HomePage.cpp
    src/app/pages/InsuredEditPage.cpp
    src/app/pages/PayerEditPage.cpp
    src/app/pages/PersonelPage.cpp
    src/app/pages/RemoveHistoryPage.cpp
    src/app/pages/RemoveInsuredPage.cpp
    src/app/pages/ReportFormPage.cpp
    src/app/pages/ReportIdEditPage.cpp
    src/app/pages/ReportIdEditPage.ui
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})

qt_add_library(pages STATIC ${SRCS})

target_compile_options(pages PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
target_link_options(pages PUBLIC ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_link_libraries(pages PUBLIC controls)
target_include_directories(pages
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(pages PROPERTIES FOLDER lib)

# #################################################################
# #  INSTALL
# #################################################################
if(QUICK_DRA_INSTALL)
    install(TARGETS pages COMPONENT libraries)

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

    add_project_test(pages ${FULL_TEST_SRCS})
    target_link_libraries(pages-test PUBLIC GTest::gmock_main pages)
endif()
