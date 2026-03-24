cmake_minimum_required(VERSION 3.19)

project("EngineMath")

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    "Private/*.cpp"
    "Private/*.hpp"
    "Public/*.hpp"
    "*.hpp"
)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})
add_library(Engine::Math ALIAS ${PROJECT_NAME})

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

find_package(GTest QUIET)

if(GTest_FOUND OR GTEST_FOUND)
    enable_testing()

    file(GLOB TEST_SOURCES "Tests/*.cpp")

    if(TEST_SOURCES)
        add_executable(${PROJECT_NAME}_test ${TEST_SOURCES})

        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/pch.hpp")
            target_precompile_headers(${PROJECT_NAME}_test PRIVATE "pch.hpp")
        endif()

        target_include_directories(${PROJECT_NAME}_test
                PUBLIC Public
                PRIVATE Private
        )

        target_link_libraries(${PROJECT_NAME}_test
                PRIVATE
                ${PROJECT_NAME}
                GTest::gtest_main
                GTest::gtest
                glm::glm
        )

        add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
    endif()
else()
    message(STATUS "GTest not found")
endif()
