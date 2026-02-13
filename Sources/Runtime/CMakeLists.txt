cmake_minimum_required(VERSION 3.19)

project("Runtime")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
	"src/*.cpp"
	"src/*.hpp"
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror)

target_link_libraries(${PROJECT_NAME}
	PRIVATE
		Module_Base
		Module_Math
		Module_Data
		Module_Engine
        Module_ResourceManager
)

install(TARGETS ${PROJECT_NAME}
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin
	PUBLIC_HEADER DESTINATION include)
