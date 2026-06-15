# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

if(TARGET Qt6::Gui)
    message(STATUS "Will also build Quick-DRA GUI")

    set(CMAKE_AUTORCC ON)

    add_subdirectory(testing)
    add_subdirectory(utils)
    add_subdirectory(gui)
    add_subdirectory(controls)
    add_subdirectory(pages)
    add_subdirectory(main)

    set_target_properties(utils gui controls pages PROPERTIES FOLDER lib/gui)
else()
    message(STATUS "Will NOT build Quick-DRA GUI")
endif()

add_to_coverage(
    gui
    utils
    controls
    pages
)
cov_set_parent_scope()
