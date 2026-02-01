include(flow_webidl)

set(SRCS
    include/quick_dra/models/count_loaded.hpp
    include/quick_dra/models/model.hpp
    include/quick_dra/models/project_reader.hpp
    include/quick_dra/models/types.hpp
    include/quick_dra/models/utility_types.hpp
    src/models/compiler.cpp
    src/models/parser_debug.cpp
    src/models/parser_impl.cpp
    src/models/project_reader.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})

add_library(libmodels STATIC ${SRCS})

add_webidl_generation(libmodels idl/models.json)

target_compile_options(libmodels PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
target_link_options(libmodels PUBLIC ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_include_directories(libmodels
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/src
)
target_link_libraries(libmodels PUBLIC libbase libpersist)

# #################################################################
# #  INSTALL
# #################################################################
if(QUICK_DRA_INSTALL)
    install(TARGETS libmodels COMPONENT libraries)

    install(
        DIRECTORY
        include/
        ${CMAKE_CURRENT_BINARY_DIR}/include/
        TYPE INCLUDE
        COMPONENT libraries
    )
endif()
