# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

option(QUICK_DRA_WIN_GUI_CONSOLE "Subsystem for the qdra-gui on Windows should default to CONSOLE" OFF)

qt_standard_project_setup()

set(SRCS
    src/main.cpp
    src/options.hpp
    src/gui_tool.cpp
    src/options.cpp
)

set(QRC_SRCS res/quick_dra.qrc)

if(WIN32)
    list(APPEND QRC_SRCS res/icon_win32.qrc)
    list(APPEND SRCS res/qdra-gui.exe.manifest res/version.rc)
else()
    list(APPEND QRC_SRCS res/icon_svg.qrc)
endif()

set(CMAKE_AUTORCC ON)
qt_add_resources(RESOURCES ${QRC_SRCS})
message(STATUS "Generated resource files: ${RESOURCES}")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS} ${QRC_SRCS})
source_group(TREE ${CMAKE_CURRENT_BINARY_DIR} PREFIX gen FILES ${RESOURCES})

qt_add_executable(qdra-gui MACOSX_BUNDLE ${SRCS} ${QRC_SRCS} ${RESOURCES})

add_win32_icon(qdra-gui appicon.ico)
target_compile_options(qdra-gui PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
target_include_directories(qdra-gui PRIVATE src)
target_link_options(qdra-gui PRIVATE ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_link_libraries(qdra-gui PRIVATE pages)
set_target_properties(qdra-gui PROPERTIES
    VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
    VS_DPI_AWARE ON
)

if(WIN32)
    if(QUICK_DRA_WIN_GUI_CONSOLE)
        message(STATUS "winapi: Build Quick-DRA GUI as Console app")
        target_link_options(qdra-gui PRIVATE /ENTRY:wmainCRTStartup)
    else()
        message(STATUS "winapi: Build Quick-DRA GUI as Windows app")
        target_link_options(qdra-gui PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:wWinMainCRTStartup)
    endif()

    target_compile_options(qdra-gui PRIVATE /D_UNICODE /DUNICODE)
endif()

# #################################################################
# #  INSTALL
# #################################################################
if(QUICK_DRA_INSTALL)
    install(TARGETS qdra-gui COMPONENT C002_gui)

    qt_generate_deploy_app_script(
        TARGET qdra-gui
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )

    install(SCRIPT "${deploy_script}" COMPONENT C003_qt_deployment)
endif()
