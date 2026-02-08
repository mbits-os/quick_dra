# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

set(SRCS
    src/args_parser.hpp
    src/builtins.cpp
    src/builtins.hpp
    src/commands.cpp
    src/commands.hpp
    src/insured/add/add_conversation.cpp
    src/insured/add/add_conversation.hpp
    src/insured/add/add_command.cpp
    src/insured/insured_command.cpp
    src/insured/edit/edit_command.cpp
    src/insured/list/list_command.cpp
    src/insured/remove/remove_command.cpp
    src/main.cpp
    src/payer/payer_conversation.cpp
    src/payer/payer_conversation.hpp
    src/payer/payer_command.cpp
    src/root/root_command.cpp
    src/xml/cli_options.cpp
    src/xml/cli_options.hpp
    src/xml/xml_command.cpp
    src/xml/ctre_parsing.cpp
    src/xml/ctre_parsing.hpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})

add_executable(qdra ${SRCS})

add_win32_icon(qdra appicon.ico)
target_compile_options(qdra PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
target_include_directories(qdra PRIVATE src)
target_link_options(qdra PUBLIC ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_link_libraries(qdra libconv mbits::args ctre::ctre)
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

# #################################################################
# #  TESTING
# #################################################################
if(QUICK_DRA_TESTING)
    string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_PRESET)

    foreach(TEST_SET
        001-basic
        002-xml
        003-payer
        004-insured-add
    )
        add_test(
            NAME qdra-runner-${TEST_SET}
            COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/.flow/flow.py"
                tools "test-runner"
                --preset "${CMAKE_PRESET}"
                --tests "main/${TEST_SET}"
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        )
    endforeach()
endif()
