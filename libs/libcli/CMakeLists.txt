# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

set(SRCS
    include/quick_dra/cli/builtins.hpp
    include/quick_dra/cli/commands.hpp
    src/builtins.cpp
    src/commands.cpp
    src/insured/insured_add_command.cpp
    src/insured/insured_add_conversation.cpp
    src/insured/insured_add_conversation.hpp
    src/insured/insured_command.cpp
    src/insured/insured_edit_command.cpp
    src/insured/insured_edit_conversation.cpp
    src/insured/insured_edit_conversation.hpp
    src/insured/insured_list_command.cpp
    src/insured/insured_remove_command.cpp
    src/insured/insured_remove_options.cpp
    src/insured/insured_remove_options.hpp
    src/payer/payer_command.cpp
    src/payer/payer_conversation.cpp
    src/payer/payer_conversation.hpp
    src/root/root_command.cpp
    src/xml/cli_options.cpp
    src/xml/cli_options.hpp
    src/xml/ctre_parsing.cpp
    src/xml/ctre_parsing.hpp
    src/xml/xml_command.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})

add_library(libcli STATIC ${SRCS})

target_compile_options(libcli PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
target_link_options(libcli PUBLIC ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_link_libraries(libcli libconv ctre::ctre)
target_include_directories(libcli
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# #################################################################
# #  INSTALL
# #################################################################
if(QUICK_DRA_INSTALL)
    install(TARGETS libcli COMPONENT libraries)

    install(
        DIRECTORY include/
        TYPE INCLUDE
        COMPONENT libraries
    )
endif()
