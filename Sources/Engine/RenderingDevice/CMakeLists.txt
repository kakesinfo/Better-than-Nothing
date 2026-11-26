cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project("EngineRenderingDevice")

find_package(Vulkan REQUIRED)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    "Private/*.cpp"
    "Private/*.hpp"
    "Public/*.hpp"
    "*.hpp"
)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})
add_library(Engine::RenderingDevice ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC Public
    PRIVATE Private
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Engine::Common
        Vulkan::Headers
    PRIVATE
        Engine::Data
        Engine::Math
        Engine::Platform
        Engine::RenderingApi
)

target_precompile_headers(${PROJECT_NAME} PRIVATE "pch.hpp")

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

if(TARGET GTest::gtest_main)
    enable_testing()
    file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "Tests/*.cpp")
    if(TEST_SOURCES)
        add_executable(${PROJECT_NAME}_test ${TEST_SOURCES})
        target_include_directories(${PROJECT_NAME}_test
            PUBLIC Public
            PRIVATE Private
        )
        target_link_libraries(${PROJECT_NAME}_test
            PRIVATE
                ${PROJECT_NAME}
                Engine::RenderingApi
                GTest::gtest_main
        )
        add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
    endif()
endif()
