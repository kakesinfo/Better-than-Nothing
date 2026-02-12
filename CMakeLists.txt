# ==================================================================================================
# Global configuration
# ==================================================================================================
cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

project("better-than-nothing" VERSION 0.3.0 LANGUAGES C CXX)

# ==================================================================================================
# Dependencies
# ==================================================================================================
add_subdirectory("ThirdParty/sdl")
add_subdirectory("ThirdParty/vulkan")
add_subdirectory("ThirdParty/vma")
add_subdirectory("ThirdParty/glm")
add_subdirectory("ThirdParty/glslang")
add_subdirectory("ThirdParty/spirv_cross")
add_subdirectory("ThirdParty/stb")
add_subdirectory("ThirdParty/imgui")

# ==================================================================================================
# Modules
# ==================================================================================================
add_subdirectory("Sources/Modules/Base")
add_subdirectory("Sources/Modules/Math")
add_subdirectory("Sources/Modules/Data")
add_subdirectory("Sources/Modules/Platform")
add_subdirectory("Sources/Modules/RenderingApi")
add_subdirectory("Sources/Modules/Renderer")
add_subdirectory("Sources/Modules/ResourceManager")
add_subdirectory("Sources/Modules/Engine")

# ==================================================================================================
# Sources
# ==================================================================================================
add_subdirectory("Sources/Runtime")
