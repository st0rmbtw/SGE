cmake_minimum_required(VERSION 3.18)

project(SGE LANGUAGES CXX C)

cmake_policy(SET CMP0069 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)

if(APPLE)
    enable_language(OBJCXX)
    set(CMAKE_OBJCXX_STANDARD 20)
endif()

find_package(Python REQUIRED COMPONENTS Interpreter)

option(SGE_BUILD_STATIC "Build SGE as static library" ON)
option(SGE_BUILD_EXAMPLES "Build examples" OFF)
option(SGE_ENABLE_PROFILE "Enable profiling via Tracy" OFF)
option(SGE_BACKEND_VULKAN "Enable the Vulkan backend" OFF)

set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")

if (WIN32)
    add_compile_definitions(NOMINMAX)
endif()

if (MSVC)
    add_compile_options(/wd5240)
else()
    add_compile_options(-Wno-ignored-attributes -march=native)
endif()

add_compile_definitions(GLM_FORCE_INTRINSICS)

set(LLGL_BUILD_RENDERER_NULL OFF CACHE BOOL "" FORCE)

if (APPLE)
    set(LLGL_BUILD_RENDERER_METAL ON CACHE BOOL "" FORCE)
else()
    set(LLGL_BUILD_RENDERER_METAL OFF CACHE BOOL "" FORCE)
    set(LLGL_VK_ENABLE_SPIRV_REFLECT ON CACHE BOOL "" FORCE)
endif()

if (SGE_BACKEND_VULKAN)
    set(LLGL_BUILD_RENDERER_VULKAN ON CACHE BOOL "" FORCE)
endif()

if (WIN32)
    set(LLGL_BUILD_RENDERER_DIRECT3D12 ON CACHE BOOL "" FORCE)
    set(LLGL_BUILD_RENDERER_DIRECT3D11 ON CACHE BOOL "" FORCE)
    set(LLGL_D3D11_ENABLE_FEATURELEVEL "Direct3D 11.1" CACHE STRING "" FORCE)
else()
    set(LLGL_BUILD_RENDERER_DIRECT3D12 OFF CACHE BOOL "" FORCE)
    set(LLGL_BUILD_RENDERER_DIRECT3D11 OFF CACHE BOOL "" FORCE)
endif()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)

if(SGE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

file(GLOB_RECURSE SGE_SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.mm")
if (NOT APPLE)
    set_source_files_properties(${SGE_SRC} PROPERTIES COMPILE_FLAGS "-x c++")
endif()

if(SGE_BUILD_STATIC)
    add_library(${PROJECT_NAME} STATIC)
else()
    add_library(${PROJECT_NAME} SHARED)
endif()

if(SGE_ENABLE_PROFILE)
    set(TRACY_ENABLE CACHE BOOL "" ON)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SGE_PROFILE_ENABLED=1)
    target_link_libraries(${PROJECT_NAME} PUBLIC TracyClient)
endif()

add_subdirectory(thirdparty/LLGL)
add_subdirectory(thirdparty/glm)
add_subdirectory(thirdparty/glfw)
add_subdirectory(thirdparty/STB)
add_subdirectory(thirdparty/fmt)
add_subdirectory(thirdparty/tracy)
add_subdirectory(thirdparty/freetype-2.13.2)

target_sources(${PROJECT_NAME} PRIVATE ${SGE_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/include")

if(NOT MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE "-Wall" "-Wextra" "-Wno-gnu-zero-variadic-macro-arguments")
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE "/Zc:preprocessor")
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC "$<$<CONFIG:Debug>:SGE_DEBUG>")

target_link_libraries(${PROJECT_NAME} PUBLIC glm STB glfw fmt LLGL freetype)

# ========================= Embed Shaders =========================

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    file(GLOB_RECURSE SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/renderer/shaders/*.*)

    foreach(shader ${SHADERS})
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/renderer/shaders.hpp
            COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/embed_shaders.py ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS ${shader}
        )
    endforeach()

    add_custom_target(EmbedShaders ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/renderer/shaders.hpp)
endif()
