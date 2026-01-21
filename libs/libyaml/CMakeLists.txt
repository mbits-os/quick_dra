set(SRCS
    include/yaml/parser.hpp
    include/yaml/reader.hpp
    include/yaml/ref.hpp
    src/parser.cpp
    src/reader.cpp
    src/ref.cpp

    # include/quick_dra/yaml/base_types.hpp
    # include/quick_dra/yaml/model.hpp
    # include/quick_dra/yaml/types.hpp
    # src/models/_parser_types.hpp
    # src/models/base_types.cpp
    # src/models/compiler.cpp
    # src/models/parser_debug.cpp
    # src/models/parser_impl.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})

add_library(libyaml STATIC ${SRCS})

target_compile_options(libyaml PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
target_link_options(libyaml PUBLIC ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_include_directories(libyaml
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/src
)
target_link_libraries(libyaml PUBLIC ryml::ryml fmt::fmt)

# #################################################################
# #  INSTALL
# #################################################################
if(QUICK_DRA_INSTALL)
    install(TARGETS libyaml COMPONENT libraries)

    install(
        DIRECTORY
        include/
        ${CMAKE_CURRENT_BINARY_DIR}/include/
        TYPE INCLUDE
        COMPONENT libraries
    )
endif()
