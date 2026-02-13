# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

add_subdirectory(libpersist)
add_subdirectory(libbase)
add_subdirectory(libmodels)
add_subdirectory(liblex)
add_subdirectory(libforms)
add_subdirectory(libconv)
add_subdirectory(cli)

add_to_coverage(
    libpersist
    libbase
    libmodels
    liblex
    libforms
    libconv
    cli
)
cov_set_parent_scope()
