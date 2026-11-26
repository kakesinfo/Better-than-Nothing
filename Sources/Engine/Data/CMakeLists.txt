cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project("EngineData")

add_library(${PROJECT_NAME} INTERFACE)
add_library(Engine::Data ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    INTERFACE Public
)

target_link_libraries(${PROJECT_NAME}
    INTERFACE
        Engine::Common
        Engine::Math
)

install(DIRECTORY Public/ DESTINATION include)

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
)

if(TARGET GTest::gtest_main)
    enable_testing()
    file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "Tests/*.cpp")
    if(TEST_SOURCES)
        add_executable(${PROJECT_NAME}_test ${TEST_SOURCES})
        target_link_libraries(${PROJECT_NAME}_test
            PRIVATE
                ${PROJECT_NAME}
                GTest::gtest_main
        )
        add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
    endif()
endif()
