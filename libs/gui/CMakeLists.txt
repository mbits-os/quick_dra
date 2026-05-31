# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

if (TARGET Qt6::Gui)
add_subdirectory(utils)
add_subdirectory(gui)
add_subdirectory(main)
add_subdirectory(app)

set_target_properties(utils gui main PROPERTIES FOLDER lib/gui)
endif()
