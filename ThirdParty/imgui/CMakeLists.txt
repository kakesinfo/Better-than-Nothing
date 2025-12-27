cmake_minimum_required(VERSION 3.16)

project(imgui)

find_package(X11 REQUIRED)

set(SOURCES
		"imgui.cpp"
		"imgui_draw.cpp"
		"imgui_demo.cpp"
		"imgui_tables.cpp"
		"imgui_widgets.cpp"
		"imgui_impl_sdl3.cpp"
		"imgui_impl_vulkan.cpp"
		"include/imgui.h"
		"include/imconfig.h"
		"include/imgui_internal.h"
		"include/imgui_impl_sdl3.h"
		"include/imgui_impl_vulkan.h"
		"include/imstb_rectpack.h"
		"include/imstb_textedit.h"
		"include/imstb_truetype.h"
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC "include")

target_link_libraries(${PROJECT_NAME} PUBLIC
		SDL3::SDL3 ${X11_LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES
		POSITION_INDEPENDENT_CODE ON
)





