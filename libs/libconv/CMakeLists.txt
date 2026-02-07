set(SRCS
    include/quick_dra/conv/concepts.hpp
    include/quick_dra/conv/conversation.hpp
    include/quick_dra/conv/field_policy.hpp
    include/quick_dra/conv/interactive.hpp
    include/quick_dra/conv/low_level.hpp
    include/quick_dra/conv/validators.hpp
    src/low_level.cpp
    src/validators.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})

add_library(libconv STATIC ${SRCS})

target_compile_options(libconv PRIVATE ${QUICK_DRA_ADDITIONAL_COMPILE_FLAGS})
target_link_options(libconv PUBLIC ${QUICK_DRA_ADDITIONAL_LINK_FLAGS})
target_include_directories(libconv
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(libconv PUBLIC libforms)

# #################################################################
# #  INSTALL
# #################################################################
if(QUICK_DRA_INSTALL)
    install(TARGETS libconv COMPONENT libraries)

    install(
        DIRECTORY include/
        TYPE INCLUDE
        COMPONENT libraries
    )
endif()
