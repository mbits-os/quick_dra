set(WEBUI_BUILD_EXAMPLES OFF)

add_subdirectory(args)
add_subdirectory(webui)

set_target_properties(args webui PROPERTIES FOLDER lib/external)
