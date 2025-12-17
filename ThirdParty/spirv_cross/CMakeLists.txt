cmake_minimum_required(VERSION 3.16)

project(spirv_cross)

set(SOURCES
	"spirv_cross/spirv_cfg.cpp"
	"spirv_cross/spirv_cpp.cpp"
	"spirv_cross/spirv_cross_c.cpp"
	"spirv_cross/spirv_cross_parsed_ir.cpp"
	"spirv_cross/spirv_cross_util.cpp"
	"spirv_cross/spirv_cross.cpp"
	"spirv_cross/spirv_glsl.cpp"
	"spirv_cross/spirv_parser.cpp"
	"spirv_cross/spirv_reflect.cpp"
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties(${PROJECT_NAME} PROPERTIES
		POSITION_INDEPENDENT_CODE ON
)