cmake_minimum_required(VERSION 3.19)

project(tinygltf)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(tinygltf)
target_sources(tinygltf PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tiny_gltf.cc)
target_include_directories(tinygltf
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
