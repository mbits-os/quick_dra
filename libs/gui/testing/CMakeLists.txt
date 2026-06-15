# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

qt_standard_project_setup()

set(SRCS
    include/palette_override.hpp
    include/stamper.hpp
    include/test_offscreen.hpp
    src/stamper.cpp
    src/test_offscreen.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})

qt_add_library(testing STATIC ${SRCS})

target_compile_options(testing PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
target_link_options(testing PUBLIC ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_link_libraries(testing PUBLIC Qt6::Test Qt6::Gui Qt6::Widgets libbase)
target_include_directories(testing
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
