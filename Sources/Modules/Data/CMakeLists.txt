cmake_minimum_required(VERSION 3.19)

project(Module_Data)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
    INTERFACE Public
)

target_link_libraries(${PROJECT_NAME}
    INTERFACE
        Module_Base
        Module_Math
)

install(DIRECTORY Public/ DESTINATION include)

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
)