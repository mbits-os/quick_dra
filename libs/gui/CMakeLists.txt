# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

option(QUICK_DRA_WIN_GUI_CONSOLE "Subsystem for the qdra-gui on Windows should default to CONSOLE" OFF)

set(SRCS
    src/main.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})

add_executable(qdra-gui ${SRCS})

add_win32_icon(qdra-gui appicon.ico)
target_compile_options(qdra-gui PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
target_include_directories(qdra-gui PRIVATE src)
target_link_options(qdra-gui PRIVATE ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_link_libraries(qdra-gui PRIVATE libgui)
set_target_properties(qdra-gui PROPERTIES
    VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
    VS_DPI_AWARE ON
)

if(WIN32)
    if (QUICK_DRA_WIN_GUI_CONSOLE)
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
    install(TARGETS qdra COMPONENT cli)

    install(
        DIRECTORY ${PROJECT_BINARY_DIR}/share/
        DESTINATION share/
        USE_SOURCE_PERMISSIONS
        COMPONENT cli
    )

    cpack_add_component(cli
        DISPLAY_NAME "Quick-DRA CLI Application"
        DESCRIPTION "Command line interface `qdra` application and data"
        GROUP apps
    )
endif()
