include(${CMAKE_CURRENT_LIST_DIR}/config.cmake)
include(CPackComponent)

cpack_add_component_group(apps
    DISPLAY_NAME Aplikacje
    DESCRIPTION "Narzędzia dla użytkowników końcowych"
    EXPANDED
)

if (TARGET Qt6::Gui)
    cpack_add_component_group(qt6
        DESCRIPTION "Runtime for the Quick-DRA GUI application"
    )
endif()

cpack_add_component(C001_cli
    DISPLAY_NAME "Aplikacja CLI"
    DESCRIPTION "Interfejs wiersza poleceń aplikacji `qdra`"
    DEPENDS C004_data
    GROUP apps
)

if (TARGET Qt6::Gui)
    cpack_add_component(C002_gui
        DISPLAY_NAME "Aplikacja GUI"
        DESCRIPTION "Graficzny interfejs użytkownika aplikacji `qdra-gui`"
        DEPENDS C004_data C003_qt_deployment
        GROUP apps
    )

    cpack_add_component(C003_qt_deployment
        DISPLAY_NAME "Środowisko Qt6"
        DESCRIPTION "Biblioteki czasu wykonania Qt6 dla aplikacji Quick-DRA GUI"
        # DOWNLOADED
        GROUP qt6
    )
endif()

cpack_add_component(C004_data
    DISPLAY_NAME "Pliki danych"
    DESCRIPTION "Pliki konfiguracyjne i zasoby dla aplikacji Quick-DRA"
    GROUP apps
)

if (TARGET Qt6::Gui)
    cpack_configure_downloads(
        "https://github.com/mbits-os/quick_dra/releases/download/v${PROJECT_VERSION}/"
        UPLOAD_DIRECTORY "${PROJECT_BINARY_DIR}/packages/upload"
        ADD_REMOVE
    )
endif()

