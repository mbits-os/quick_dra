# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

qt_standard_project_setup()

set(SRCS
    include/app/main/MainWindow.hpp
    src/app/main/MainWindow.cpp
    src/app/main/MainWindowUi.cpp

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
    install(TARGETS pages COMPONENT gui_libraries)

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
        tests/MainWindow.test.cpp
        tests/PagesTest.cpp
        tests/PagesTest.hpp
        tests/PagesTest.verify.HomePage.cpp
        tests/PagesTest.verify.InsuredEditPage.cpp
        tests/PagesTest.verify.PayerEditPage.cpp
        tests/PagesTest.verify.PersonelPage.cpp
        tests/PagesTest.verify.RemoveInsuredPage.cpp
        tests/PagesTest.verify.ReportFormPage.cpp
        tests/PagesTest.verify.ReportIdEditPage.cpp
    )

    get_target_property(QDRA_SOURCE_DIR qdra SOURCE_DIR)
    configure_file(tests/TestPaths.hpp.in TestPaths.hpp @ONLY)

    set(FULL_TEST_GEN_SRCS
        "${CMAKE_CURRENT_BINARY_DIR}/TestPaths.hpp"
    )

    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/tests FILES ${FULL_TEST_SRCS})
    source_group(TREE ${CMAKE_CURRENT_BINARY_DIR} PREFIX gen FILES ${FULL_TEST_GEN_SRCS})

    qt_add_project_test(pages ${FULL_TEST_SRCS} ${FULL_TEST_GEN_SRCS})
    target_link_libraries(pages-test PUBLIC testing libcli pages)
endif()
