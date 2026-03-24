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
# Engine Dependencies
# ==================================================================================================
add_subdirectory("ThirdParty/sdl")
add_subdirectory("ThirdParty/vulkan")
add_subdirectory("ThirdParty/vma")
add_subdirectory("ThirdParty/glm")
add_subdirectory("ThirdParty/glslang")
add_subdirectory("ThirdParty/spirv_cross")
add_subdirectory("ThirdParty/stb")
add_subdirectory("ThirdParty/imgui")
add_subdirectory("ThirdParty/entt")
add_subdirectory("ThirdParty/meshoptimizer")
add_subdirectory("ThirdParty/tinygltf")

# ==================================================================================================
# Engine Modules
# ==================================================================================================
add_subdirectory("Sources/Engine/Common")
add_subdirectory("Sources/Engine/Math")
add_subdirectory("Sources/Engine/Data")
add_subdirectory("Sources/Engine/Scene")
add_subdirectory("Sources/Engine/Platform")
add_subdirectory("Sources/Engine/RenderingApi")
add_subdirectory("Sources/Engine/RenderingDevice")
add_subdirectory("Sources/Engine/Editor")
add_subdirectory("Sources/Engine/Renderer")
add_subdirectory("Sources/Engine/ResourceImporter")
add_subdirectory("Sources/Engine/ResourceManager")
add_subdirectory("Sources/Engine/Core")

# ==================================================================================================
# Applications
# ==================================================================================================
add_subdirectory("Sources/Runtime")
