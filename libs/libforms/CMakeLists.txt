# Copyright (c) 2026 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

set(SRCS
    include/quick_dra/docs/file_set.hpp
    include/quick_dra/docs/forms.hpp
    include/quick_dra/docs/summary.hpp
    include/quick_dra/docs/xml_builder.hpp
    include/quick_dra/docs/xml.hpp
    include/quick_dra/io/http.hpp
    include/quick_dra/io/options.hpp
    src/docs/file_set.cpp
    src/docs/forms.cpp
    src/docs/summary.cpp
    src/docs/xml_builder.cpp
    src/docs/xml.cpp
    src/io/http.cpp
    src/io/options.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})
add_library(libforms STATIC ${SRCS})

target_compile_options(libforms PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
target_link_options(libforms PUBLIC ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_include_directories(libforms
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(libforms PUBLIC CURL::libcurl libmodels liblex)

add_custom_command(
    TARGET libforms POST_BUILD
    COMMENT "Copy config files"
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy_directory "${PROJECT_SOURCE_DIR}/data/config"
    "${PROJECT_BINARY_DIR}/${SHARE_DIR}/config")

add_custom_command(
    TARGET libforms POST_BUILD
    COMMENT "Copy schema files"
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy_directory "${PROJECT_SOURCE_DIR}/data/schemas"
    "${PROJECT_BINARY_DIR}/${SHARE_DIR}/schemas")

# #################################################################
# #  INSTALL
# #################################################################
if(QUICK_DRA_INSTALL)
    install(TARGETS libforms COMPONENT libraries)

    install(
        DIRECTORY include/
        TYPE INCLUDE
        COMPONENT libraries
    )
endif()

# #################################################################
# #  TESTING
# #################################################################
if(QUICK_DRA_TESTING)
    add_subdirectory(tests/mock_curl)

    add_library(libforms_tested STATIC ${SRCS})

    target_compile_options(libforms_tested PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
    target_link_options(libforms_tested PUBLIC ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
    target_include_directories(libforms_tested
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    target_link_libraries(libforms_tested PUBLIC mock_curl libmodels liblex)

    file(GLOB FORMS_TEST_SRCS_CC tests/*.cc)
    file(GLOB FORMS_TEST_SRCS_CPP tests/*.cpp)
    file(GLOB FORMS_TEST_SRCS_CXX tests/*.cxx)
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/tests FILES
        ${FORMS_TEST_SRCS_CC}
        ${FORMS_TEST_SRCS_CPP}
        ${FORMS_TEST_SRCS_CXX})

    add_project_test(libforms ${FORMS_TEST_SRCS_CC} ${FORMS_TEST_SRCS_CPP} ${FORMS_TEST_SRCS_CXX})
    target_link_libraries(libforms-test PUBLIC GTest::gmock_main libforms_tested)
endif()
