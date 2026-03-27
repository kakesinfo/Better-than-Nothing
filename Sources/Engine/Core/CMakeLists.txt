cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project("EngineCore")

find_package(Vulkan REQUIRED)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    "Private/*.cpp"
    "Private/*.hpp"
    "Public/*.hpp"
    "*.hpp"
)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})
add_library(Engine::Core ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC Public
    PRIVATE Private
)

target_precompile_headers(${PROJECT_NAME} PRIVATE "pch.hpp")

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Engine::RenderingApi
        Engine::RenderingDevice
        Engine::Renderer
        Engine::Editor
    PUBLIC
        Engine::Common
        Engine::Data
        Engine::Math
        Engine::Platform
        Engine::Scene
        Engine::ResourceManager
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
