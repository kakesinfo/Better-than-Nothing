cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project("EngineRenderSettings")

add_library(${PROJECT_NAME} INTERFACE)
add_library(Engine::RenderSettings ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    INTERFACE Public
)

target_link_libraries(${PROJECT_NAME}
    INTERFACE
        Engine::Common
)

install(DIRECTORY Public/ DESTINATION include)

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
)
