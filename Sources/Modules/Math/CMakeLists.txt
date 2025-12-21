cmake_minimum_required(VERSION 3.19)

project(Module_Math)

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

target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)

target_precompile_headers(${PROJECT_NAME} PRIVATE "pch.hpp")

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:RELEASE>:-O3>
)

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

####################################################################################################
# Google Test configuration
####################################################################################################

enable_testing()

include(FetchContent)

set(FETCH_ARGS "")
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.24)
    # Cette option n'existe qu'à partir de la version 3.24
    list(APPEND FETCH_ARGS DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
endif()

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        ${FETCH_ARGS}
)

FetchContent_MakeAvailable(googletest)

file(GLOB TEST_SOURCES "Tests/*.cpp")

if(TEST_SOURCES)
    add_executable(${PROJECT_NAME}_test ${TEST_SOURCES})

    target_precompile_headers(${PROJECT_NAME}_test PRIVATE "pch.hpp")

    target_include_directories(${PROJECT_NAME}_test
            PUBLIC Public
            PRIVATE Private
    )

    target_link_libraries(${PROJECT_NAME}_test PRIVATE ${PROJECT_NAME} GTest::gtest_main PUBLIC glm::glm)

    add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
endif()