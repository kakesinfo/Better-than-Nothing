cmake_minimum_required(VERSION 3.16)

project(VulkanMemoryAllocator)

set(SOURCES
    "vk_mem_alloc.cpp"
    "include/vma/vk_mem_alloc.h"
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC "include")

target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Headers)

set_target_properties(${PROJECT_NAME} PROPERTIES
        POSITION_INDEPENDENT_CODE ON
)