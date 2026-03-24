cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(Module_ResourceManager)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    "Private/*.cpp"
    "Private/*.hpp"
    "Public/*.hpp"
    "*.hpp"
)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC Public
    PRIVATE Private
)

target_include_directories(${PROJECT_NAME} PRIVATE ../../../ThirdParty/tinyobjloader)

target_precompile_headers(${PROJECT_NAME} PRIVATE "pch.hpp")

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Engine::Common
        Engine::Data
        Engine::Math
        Engine::RenderingApi
        Engine::RenderingDevice
        ResourceImporter
        meshoptimizer
    PUBLIC
        glslang
        glslang-default-resource-limits
        SPIRV
        spirv_cross
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
