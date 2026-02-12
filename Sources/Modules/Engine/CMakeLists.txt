cmake_minimum_required(VERSION 3.19)

project(Module_Engine)

file(GLOB_RECURSE PROJECT_SOURCES
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

target_precompile_headers(${PROJECT_NAME} PRIVATE "pch.hpp")

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Module_Base
        Module_Math
        Module_Data
        Module_LegacyRenderer
        Module_Platform
        Module_RenderingApi
        Module_ResourceManager
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    SKIP_BUILD_RPATH FALSE
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "$ORIGIN"
    VISIBILITY_INLINES_HIDDEN ON
)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)
