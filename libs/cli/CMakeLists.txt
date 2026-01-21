# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

set(SRCS
    src/builtins.hpp
    src/commands.cpp
    src/commands.hpp
    src/main.cpp
    src/root/command.cpp
    src/xml/cli_options.cpp
    src/xml/cli_options.hpp
    src/xml/command.cpp
)

add_executable(qdra ${SRCS})

add_win32_icon(qdra appicon.ico)
target_compile_options(qdra PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
target_link_options(qdra PUBLIC ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_link_libraries(qdra libforms mbits::args)
set_target_properties(qdra PROPERTIES
    VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if(WIN32)
    target_link_options(qdra PRIVATE /ENTRY:wmainCRTStartup)
    target_compile_options(qdra PUBLIC /D_UNICODE /DUNICODE)
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
