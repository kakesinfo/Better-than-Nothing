cmake_minimum_required(VERSION 3.19)

project("EngineData")

add_library(${PROJECT_NAME} INTERFACE)
add_library(Engine::Data ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    INTERFACE Public
)

target_link_libraries(${PROJECT_NAME}
    INTERFACE
        Engine::Common
        Module_Math
)

install(DIRECTORY Public/ DESTINATION include)

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
)
