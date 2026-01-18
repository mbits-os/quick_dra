# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

configure_file(src/version.hpp.in include/quick_dra/version.hpp @ONLY)
configure_file(src/base/dir_names.hpp.in include/quick_dra/base/dir_names.hpp @ONLY)

set(GEN_SRCS
    "${CMAKE_CURRENT_BINARY_DIR}/include/quick_dra/version.hpp"
    "${CMAKE_CURRENT_BINARY_DIR}/include/quick_dra/base/dir_names.hpp"
)

set(SRCS
    include/quick_dra/base/meta.hpp
    include/quick_dra/base/paths.hpp
    include/quick_dra/base/str.hpp
    include/quick_dra/base/verbose.hpp
    src/base/paths.cpp
    src/base/str.cpp
)

if(UNIX)
    list(APPEND SRCS src/base/posix.cpp)
elseif(WIN32)
    list(APPEND SRCS src/base/win32.cpp)
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})
source_group(TREE ${CMAKE_CURRENT_BINARY_DIR} PREFIX gen FILES ${GEN_SRCS})

add_library(libbase STATIC ${GEN_SRCS} ${SRCS})

target_compile_options(libbase PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
target_link_options(libbase PUBLIC ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_include_directories(libbase
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/src
)
target_link_libraries(libbase PUBLIC fmt::fmt)

# #################################################################
# #  INSTALL
# #################################################################
if(QUICK_DRA_INSTALL)
    install(TARGETS libbase COMPONENT libraries)

    install(
        DIRECTORY
        include/
        ${CMAKE_CURRENT_BINARY_DIR}/include/
        TYPE INCLUDE
        COMPONENT libraries
    )

    cpack_add_component(libraries
        DISPLAY_NAME "Quick-DRA Libraries"
        DESCRIPTION "Library and include files"
        DISABLED
        GROUP devel
    )
endif()
