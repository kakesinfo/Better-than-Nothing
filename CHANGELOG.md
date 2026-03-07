# Changelog

All notable changes to this project will be documented in this file.

## [v0.4.0] - 2026-03-07
- 📝 Updated README and CHANGELOG for v0.4.0
- 🎨 Removed all legacy Vulkan-prefixed classes and VulkanContext
- ⚡ Introducing a permanent 128MB Staging Buffer
- ⚡ Compressed sponza and modular-ruins assets
- 🎨 Removed Forward Renderpass and RenderGraph abstraction
- ⚡ Improved DrawStream sorting method
- 🚚 Moved frame logic to RenderingDevice
- 🚚 Moved CommandBuffers, Semaphores and Fences to RenderingDevice
- 🚚 Moved VertexInputDescription
- 🎨 Splitted RenderingDevice.cpp in multiple files
- ♻️ Removed all Registry classes to put the logic inside RenderingDevice
- 🔥 Removed TextureLoader from ResourceManager
- ♻️ Handles are now 64-bits, template-less and contain the handle type
- ✨ Introducing DescriptorSet Registry
- ✨ Implemented Vector4
- ✨ Introducing TextureRegistry
- ⚡ Inline code in Math module
- 🐛 Fixed sync code in render passes
- 📄 Updated 2025 to 2026 in the copyright headers
- 🔥 Removed VulkanFrame struct
- 📝 Updated README file
- ⚡ Using BufferRegistry in ResourceManager to load meshes
- ✨ Introducing the BufferRegistry to create and destroy Buffers
- ✨ Introducing the SamplerRegistry to create, destroy and cache samplers
- ⚡ Moved imageFinished semaphore and AcquireNextImage to SwapChain
- ⚡ Reworked DrawStream with sort keys and iterator
- ✨ Introducing all-new DescriptorSetLayout implementation
- ✨ Added a RenderingApi namespace
- ✨ Added some structs to Resources folder and moved vulkan impl to new Core folder
- 🐛 Fixed a Semaphore/Queue syncing error caused by triple buffering
- ✨ Introducing a Push Constant for the Dynamic Uniforms
- ✨ Add RenderingDevice class
- 🎨 Cleaned PCH includes and cstdint use cases
- 🗑️ Deprecated all "Vulkan"-prefixed classes in RenderingApi
- 🔥 Removed BufferManager
- ⬆️ Upgraded VMA to 3.3.0 for Vulkan 1.4 compatibility
- 🏗️ Add a new module RenderingDevice
- ♻️ Refactor CommandBuffer without VulkanContext
- ♻️ Refactor DescriptorPool without VulkanContext
- 🚚 Renamed some variables that were prefixed vk
- ♻️ Refactor Fence and Semaphore without VulkanContext
- ♻️ Refactor SwapChain without VulkanContext
- ♻️ Refactor CommandPool without VulkanContext
- ♻️ Refactor Allocator without VulkanContext
- ♻️ Refactor Device and Queue without VulkanContext
- 🔨 Added CONFIGURE_DEPENDS to all CMakeLists.txt
- 🐛 Fixed an error in CMake when GTest is not found
- ♻️ Refactor Surface without VulkanContext
- ♻️ Refactor Instance without VulkanContext
- 🚚 Renamed LegacyRenderer.cpp to Renderer.cpp
- ⚡ Fixed buffering mode and V-Sync
- 🚚 Rename LegacyRenderer to Renderer
- 🚚 Remove "Vulkan" prefixes from RenderingApi vulkan implementations
- 🚚 Rename LegacyVulkan to RenderingAPI

## [v0.3.4] - 2025-12-30
- ✨ Added point lights, tone mapping and gamma correction
- 🔥 Migrated from Github to on-premises Gitlab

## [v0.3.3] - 2025-12-27
- ✅ Added tests for Matrix4
- 📈 Added metrics
- ⚡️ Added a RenderGraph with RenderPass and a DrawStream
- 💄 Implemented ImGui using a RenderPass to show some metrics
- ⚡️ Reworked the Renderer to use Vulkan 1.3 Dynamic Rendering
- 📦️ Migrated from GLFW3 to SDL3

## [v0.3.2] - 2025-12-22
- ✨ Introducing the Math module
- 🎨 Removed PlaneMesh but added a MeshFactory and MeshBuilder

## [v0.3.1] - 2025-12-20
- 🎨 Updated how engine applications are loaded by Module_Engine
- 🎨 Removed all global imports (and added some license headers)
- 🎨 Removed Hongarian notation everywhere
- 🎨 Added .editorconfig, .gitattributes, .clang-format
- 🎨 Applied Clang-format to all ./Sources files
- 🎨 Removed bits/std++.h file in every pch.hpp

## [v0.3.0] - 2025-12-17
- 🏗️ New project architecture with cmake modules, ninja compiler and mandatory clang c++20

## [v0.2.5] - 2025-12-13
- 🍱 Added sponza obj and hi-res wood texture
- 🚑️ Fixed descriptor pool size based on pool sizes
- ✨ Blinn-phong lightning with energy conservation
- ⚡️ Spenza asset can now be loaded with its materials


## [v0.2.4] - 2025-12-13
- ✨ Added a free-fly camera

## [v0.2.3] - 2025-12-12
- 🐛 Fixed models not centered or without pre-transform

## [v0.2.2] - 2025-12-12
- ✨ Now use Multisampling

## [v0.2.1] - 2025-12-12
- ✨ Textures have mipmaps now

## [v0.2.0] - 2025-12-12
- ♻️ From-scratch rewrite of the entire project
- ✨ Use vertex buffers to show the triangle
- 🔥 Removed no more used "Config" folder
- ⚡️ Improving performances using staging buffers
- ⚡️ Improving performances using Index buffers
- 🎨 Code clean-up before adding more stuff
- 📝 Updated README
- 📝 Updated the license to MIT
- 💄 Now use the Hungarian notation on files
- ⬆️ Updated glm version to support CMake from 3.6 to 3.14
- 🚑️ Fixed the glm CMakeLists file
- 🎨 Now using a Render Pass Context to secure Command Buffer commands call
- 🎨 Renamed VulkanRenderer to Renderer
- 🎨 Moved some classes from Utils to Renderer
- 🔨 Moved Linux configuration script to new Scripts folder
- 👷 Create a build-docker-image.yml workflow config
- 🦺 All-new docker image to build and test Branches via Github Actions
- 📝 Enhance README.md with emoji section headers
- 🎨 Move all vulkan stuff to renderer and add a renderer context
- 👷 Update setup-linux script
- ✨ Adding uniforms and descriptors
- ✨ Implementing a RenderQueue and RenderCache for Renderer
- 🔥 Removed old CVulkanBuffer impl and other stuff
- 🔥 Renamed struct files and changed stuff
- ⚡️ Now using 2 uniforms for global and object rendering
- ✨ Enable keyboard and mouse inputs
- ✨ Implementing VulkanImage and textures
- 🚑️ Fix a crash during window resizing
- 🎨 Cleaned nor removed some files
- ⚡️ Load and compile shaders in CRenderCache
- 🎨 VkShaderModule now has a RAII wrapper
- 🎨 Refactor of RenderCache, RenderList and Renderer to load and use resources
- ✨ Now loading .obj files and rendering up to +50k instances

## [v0.1.0] - 2024-11-03

The **v0.1.0** release was the first iteration of the engine, featuring a very basic vulkan renderer with a lot of bugs.
