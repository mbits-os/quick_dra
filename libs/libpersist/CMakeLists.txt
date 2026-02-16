set(SRCS
    include/yaml/parser.hpp
    include/yaml/reader.hpp
    include/yaml/ref.hpp
    include/yaml/writer.hpp
    src/parser.cpp
    src/reader.cpp
    src/ref.cpp
    src/writer.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})

add_library(libpersist STATIC ${SRCS})

target_compile_options(libpersist PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
target_link_options(libpersist PUBLIC ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_include_directories(libpersist
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(libpersist PUBLIC ryml::ryml fmt::fmt)

# #################################################################
# #  INSTALL
# #################################################################
if(QUICK_DRA_INSTALL)
    install(TARGETS libpersist COMPONENT libraries)

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
    file(GLOB PERSIST_TEST_SRCS_CC tests/*.cc)
    file(GLOB PERSIST_TEST_SRCS_CPP tests/*.cpp)
    file(GLOB PERSIST_TEST_SRCS_CXX tests/*.cxx)
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/tests FILES
        ${PERSIST_TEST_SRCS_CC}
        ${PERSIST_TEST_SRCS_CPP}
        ${PERSIST_TEST_SRCS_CXX})

    add_project_test(libpersist ${PERSIST_TEST_SRCS_CC} ${PERSIST_TEST_SRCS_CPP} ${PERSIST_TEST_SRCS_CXX})
    target_link_libraries(libpersist-test PUBLIC GTest::gmock_main libpersist)

    add_test(NAME libpersist COMMAND libpersist-test "--gtest_output=xml:${TEST_REPORT_DIR}/libpersist/${TEST_REPORT_FILE}")
endif()
