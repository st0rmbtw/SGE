cmake_minimum_required(VERSION 3.18)

project(SGE LANGUAGES CXX C)

cmake_policy(SET CMP0069 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 99)

if(APPLE)
    enable_language(OBJCXX)
    set(CMAKE_OBJCXX_STANDARD 23)
endif()

find_package(Python REQUIRED COMPONENTS Interpreter)

option(SGE_BUILD_STATIC "Build SGE as static library" ON)
option(SGE_BUILD_EXAMPLES "Build examples" OFF)
option(SGE_ENABLE_PROFILE "Enable profiling via Tracy" OFF)
option(SGE_ENABLE_IMGUI "Enable ImGui integration" OFF)
option(SGE_BACKEND_VULKAN "Enable the Vulkan backend" OFF)
option(SGE_DEBUG_LAYER "Enable the debug layer" OFF)
option(SGE_ENABLE_DEFAULT_FONT "Enable the default font" ON)

set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")

if(WIN32)
    add_compile_definitions(NOMINMAX)
endif()

if(MSVC)
    add_compile_options(/wd5240)
else()
    add_compile_options(-Wno-ignored-attributes -march=native)
endif()

add_compile_definitions(GLM_FORCE_INTRINSICS)

set(LLGL_BUILD_RENDERER_NULL OFF CACHE BOOL "" FORCE)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)

if(SGE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

set(SGE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/engine.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/window_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/renderer/batch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/renderer/camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/renderer/context.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/renderer/framebuffer_pool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/renderer/glfw_window.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/renderer/renderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/renderer/renderer2d.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/renderer/resource.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/time/time.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/time/timer.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/types/attributes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/types/path.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/types/texture_atlas.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/utils/text.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/utils/random.cpp
)

if(NOT APPLE)
    set_source_files_properties(${SGE_SRC} PROPERTIES COMPILE_FLAGS "-x c++")
endif()

if(SGE_BUILD_STATIC)
    add_library(${PROJECT_NAME} STATIC)
else()
    add_library(${PROJECT_NAME} SHARED)
endif()

if(APPLE)
    set(LLGL_BUILD_RENDERER_METAL ON CACHE BOOL "" FORCE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SGE_METAL_BACKEND_ENABLED=1)
else()
    set(LLGL_BUILD_RENDERER_METAL OFF CACHE BOOL "" FORCE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SGE_METAL_BACKEND_ENABLED=0)
endif()
    
if(SGE_BACKEND_VULKAN)
    set(LLGL_BUILD_RENDERER_VULKAN ON CACHE BOOL "" FORCE)
    set(LLGL_VK_ENABLE_SPIRV_REFLECT ON CACHE BOOL "" FORCE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SGE_VULKAN_BACKEND_ENABLED=1)
else()
    target_compile_definitions(${PROJECT_NAME} PUBLIC SGE_VULKAN_BACKEND_ENABLED=0)
endif()

if(SGE_DEBUG_LAYER)
    message(STATUS "[SGE] DebugLayer: Enabled")
    set(LLGL_ENABLE_DEBUG_LAYER ON CACHE BOOL "" FORCE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SGE_DEBUG_LAYER_ENABLED=1)
else()
    message(STATUS "[SGE] DebugLayer: Disabled")
    set(LLGL_ENABLE_DEBUG_LAYER OFF CACHE BOOL "" FORCE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SGE_DEBUG_LAYER_ENABLED=0)
endif()

if(WIN32)
    set(LLGL_BUILD_RENDERER_DIRECT3D12 ON CACHE BOOL "" FORCE)
    set(LLGL_BUILD_RENDERER_DIRECT3D11 ON CACHE BOOL "" FORCE)
    set(LLGL_D3D11_ENABLE_FEATURELEVEL "Direct3D 11.1" CACHE STRING "" FORCE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SGE_D3D11_BACKEND_ENABLED=1)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SGE_D3D12_BACKEND_ENABLED=1)
else()
    set(LLGL_BUILD_RENDERER_DIRECT3D12 OFF CACHE BOOL "" FORCE)
    set(LLGL_BUILD_RENDERER_DIRECT3D11 OFF CACHE BOOL "" FORCE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SGE_D3D11_BACKEND_ENABLED=0)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SGE_D3D12_BACKEND_ENABLED=0)
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC SGE_OPENGL_BACKEND_ENABLED=1)

if(SGE_ENABLE_IMGUI)
    message(STATUS "[SGE] ImGui: Enabled")

    target_compile_definitions(${PROJECT_NAME} PUBLIC SGE_IMGUI_ENABLED=1)

    list(APPEND SGE_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/imgui.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/imgui_draw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/imgui_tables.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/imgui_widgets.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/imgui_demo.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/backends/imgui_impl_glfw.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/renderer/imgui_renderer.cpp
    )
else()
    target_compile_definitions(${PROJECT_NAME} PUBLIC SGE_IMGUI_ENABLED=0)
    message(STATUS "[SGE] ImGui: Disabled")
endif()

if(SGE_ENABLE_DEFAULT_FONT)
    message(STATUS "[SGE] Default font: Enabled")
    target_compile_definitions(${PROJECT_NAME} PUBLIC SGE_DEFAULT_FONT_ENABLED=1)
    list(APPEND SGE_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/types/font.cpp
    )
else()
    message(STATUS "[SGE] Default font: Disabled")
    target_compile_definitions(${PROJECT_NAME} PUBLIC SGE_DEFAULT_FONT_ENABLED=0)
endif()

if(SGE_ENABLE_PROFILE)
    set(TRACY_ENABLE CACHE BOOL "" ON)
    set(TRACY_ON_DEMAND CACHE BOOL "" ON)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SGE_PROFILE_ENABLED=1)
    target_link_libraries(${PROJECT_NAME} PUBLIC TracyClient)
    add_subdirectory(thirdparty/tracy)
    message(STATUS "[SGE] Profiling: Enabled")
else()
    message(STATUS "[SGE] Profiling: Disabled")
endif()

add_subdirectory(thirdparty/LLGL)
add_subdirectory(thirdparty/glm)
add_subdirectory(thirdparty/glfw)
add_subdirectory(thirdparty/STB)
add_subdirectory(thirdparty/fmt)
add_subdirectory(thirdparty/freetype-2.13.2)

target_sources(${PROJECT_NAME} PRIVATE ${SGE_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui)

if(NOT MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE "-Wall" "-Wextra" "-Wno-gnu-zero-variadic-macro-arguments")
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE "/Zc:preprocessor")
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC "$<$<CONFIG:Debug>:SGE_DEBUG>")

target_link_libraries(${PROJECT_NAME} PUBLIC glm STB glfw fmt LLGL freetype)
