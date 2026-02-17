# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

set(SRCS
    curl/curl.h
    curl/mock.hpp
    curl/mock.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})
add_library(mock_curl STATIC ${SRCS})
target_compile_options(mock_curl PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
target_link_options(mock_curl PUBLIC ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_include_directories(mock_curl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mock_curl PUBLIC fmt::fmt)
