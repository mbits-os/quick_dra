# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

set(SRCS
    src/convert.cpp
    src/load.cpp
    src/main.cpp
    src/qt_model.hpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})

add_executable(qtxml-to-junit ${SRCS})
target_compile_options(qtxml-to-junit PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
target_include_directories(qtxml-to-junit PRIVATE src)
target_link_options(qtxml-to-junit PRIVATE ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_link_libraries(qtxml-to-junit PRIVATE tinyxml2::tinyxml2 args libforms)

if(WIN32)
    target_link_options(qtxml-to-junit PRIVATE /ENTRY:wmainCRTStartup)
    target_compile_options(qtxml-to-junit PRIVATE /D_UNICODE /DUNICODE)
endif()
