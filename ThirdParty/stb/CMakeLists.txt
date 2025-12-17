cmake_minimum_required(VERSION 3.16)

project(stb)

set(SOURCES
	"stb_image.cpp"
	"include/stb_image.h"
	"include/stb_image_resize.h"
	"include/stb_image_write.h"
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC "include")

target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Headers)

set_target_properties(${PROJECT_NAME} PROPERTIES
		POSITION_INDEPENDENT_CODE ON
)


