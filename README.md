# Better than Nothing
Everything is in the title, it is a better-than-nothing Game Engine.

### 🚀 Features

 - Support for Linux.
 - Use C++23 and Vulkan 1.4 features.
 - In-house GLM wrapper (Vector2, Vector3, Quaternion, Matrix4)
 - In-house JIT shader compilation.
 - Blinn-Phong forward rendering with:
   - Colored directional light
   - Multiple colored point lights (SSBO)
   - Accurate energy conservation
   - Reinhard tone mapping with exposure
   - Gamma correction
 - Render hundreds of models and textures
 - Display real-time metrics using ImGui

### 🧠 Road map

I've created a Trello board to provide some insight on what I have planned for the future of the game engine.
On this board, you'll find upcoming features, target time frames, and any emergent known issues.
Please note that at this time, the current Trello is in English only.

You can find the board [here](https://trello.com/b/pKXNk3bb/road-map).

### 🧩 Contributing

Contributions are welcome. Both new features and bug fixes. Just open a pull request.

### 📦 Dependencies


- [sdl3](https://github.com/libsdl-org/SDL): A cross-platform library designed to make it easy to write multi-media softwares.
- [vulkan](https://github.com/KhronosGroup/Vulkan-Headers): A low-overhead, cross-platform API and open standard for 3D graphics and computing.
- [vulkan memory allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator): Easy to integrate Vulkan memory allocation library that help to manage memory allocations and resource creation.
- [glslang](https://github.com/KhronosGroup/glslang): A library used for JIT shader compilation from OpenGL Shading Language (GLSL) to SPIR-V format.
- [spirv-cross](https://github.com/KhronosGroup/SPIRV-Cross): A practical tool and library for performing reflection on SPIR-V and disassembling SPIR-V back to high level languages.
- [glm](https://github.com/g-truc/glm): An header only mathematics library based on the OpenGL Shading Language (GLSL) specifications.
- [stb](https://github.com/nothings/stb): Single-file public domain (or MIT licensed) libraries for C/C++.
- [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader): A header only wavefront obj loader.
- [imgui](https://github.com/ocornut/imgui): A bloat-free Graphical User interface for C++ with minimal dependencies.

### 📜 Licence

This project is under the GNU General Public License v3.0 license - see the [LICENSE](LICENSE.md) file for details.

