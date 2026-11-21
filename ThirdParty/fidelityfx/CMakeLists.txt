cmake_minimum_required(VERSION 3.16)

project(FidelityFX VERSION 2.4.0 LANGUAGES C CXX)

add_library(${PROJECT_NAME} INTERFACE)
add_library(FidelityFX::SDK ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} SYSTEM INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/api/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/upscalers/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/denoisers/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/framegeneration/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/radiancecache/include"
)

target_link_libraries(${PROJECT_NAME} INTERFACE
    Vulkan::Headers
    "${CMAKE_CURRENT_SOURCE_DIR}/signedbin/amd_fidelityfx_loader_vk.lib"
)

set(FFX_SIGNED_DLLS
    "${CMAKE_CURRENT_SOURCE_DIR}/signedbin/amd_fidelityfx_loader_vk.dll"
    "${CMAKE_CURRENT_SOURCE_DIR}/signedbin/amd_fidelityfx_upscaler_vk.dll"
    "${CMAKE_CURRENT_SOURCE_DIR}/signedbin/amd_fidelityfx_framegeneration_vk.dll"
    "${CMAKE_CURRENT_SOURCE_DIR}/signedbin/amd_fidelityfx_denoiser_dx12.dll"
)

# Copy AMD FidelityFX signed DLLs into the runtime binary directory at configure/build time
if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    file(MAKE_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    foreach(FFX_DLL IN LISTS FFX_SIGNED_DLLS)
        if(EXISTS "${FFX_DLL}")
            file(COPY "${FFX_DLL}" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
        endif()
    endforeach()
endif()
