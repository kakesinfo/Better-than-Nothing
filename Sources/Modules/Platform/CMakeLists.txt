cmake_minimum_required(VERSION 3.19)

project(Module_Platform)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
        "Private/*.cpp"
        "Private/*.hpp"
        "Public/*.hpp"
        "*.hpp"
)

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME}
        PUBLIC Public
        PRIVATE Private
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE Module_Base Module_Math
        PUBLIC SDL3::SDL3
)

target_precompile_headers(${PROJECT_NAME} PRIVATE "pch.hpp")

set_target_properties(${PROJECT_NAME} PROPERTIES
        SKIP_BUILD_RPATH FALSE
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "$ORIGIN"
        # CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
)

install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include
)
