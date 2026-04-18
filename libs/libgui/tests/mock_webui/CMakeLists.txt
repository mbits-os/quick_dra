# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

set(SRCS
    webui/webui.cpp
    webui/webui.h
    webui/webui.hpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})
add_library(mock_webui STATIC ${SRCS})
target_compile_options(mock_webui PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
target_link_options(mock_webui PUBLIC ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_include_directories(mock_webui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/webui)
target_link_libraries(mock_webui PUBLIC fmt::fmt)
set_target_properties(mock_webui PROPERTIES FOLDER tests/utils)
