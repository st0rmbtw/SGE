cmake_minimum_required(VERSION 3.18)

project(SGE LANGUAGES CXX C)

cmake_policy(SET CMP0069 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)

if(APPLE)
    enable_language(OBJCXX)
endif()

find_package(Python REQUIRED COMPONENTS Interpreter)

option(SGE_BUILD_STATIC "Build SGE as static library" ON)
option(SGE_BUILD_EXAMPLES "Build examples" OFF)

set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(RELEASE_BUILD OFF)
else()
    set(RELEASE_BUILD ON)
endif()

if (RELEASE_BUILD)
    message("-- Build Type: Release")
    if(MSVC)
        add_compile_options("/O2" "/Ob3" "/Qfast_transcendentals" "${BITCODE}")
        STRING(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        STRING(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
        STRING(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
        STRING(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    else()
        add_compile_options("-g0" "-O3")
        add_compile_options("-fomit-frame-pointer")
        add_compile_options("-fno-math-errno" "-ffinite-math-only" "-fno-signed-zeros" "-fno-trapping-math" "-fassociative-math")
        add_compile_options("-fno-exceptions")
    endif()
    set(DISABLE_FORCE_DEBUG_POSTFIX ON)
    # set(LLGL_BUILD_STATIC_LIB ON CACHE BOOL "")
    set(LLGL_ENABLE_DEBUG_LAYER OFF CACHE BOOL "" FORCE)
    set(LLGL_ENABLE_CHECKED_CAST OFF CACHE BOOL "" FORCE)
    set(LLGL_ENABLE_EXCEPTIONS OFF CACHE BOOL "" FORCE)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message("-- Build Type: Debug")
    if(MSVC)
        add_compile_options("/DEBUG" "/Od")
    else()
        add_compile_options("-ggdb" "-O0")
    endif()
    set(DISABLE_FORCE_DEBUG_POSTFIX OFF)
    set(LLGL_ENABLE_EXCEPTIONS ON CACHE BOOL "" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

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
    set(LLGL_BUILD_RENDERER_VULKAN ON CACHE BOOL "" FORCE)
    set(LLGL_VK_ENABLE_SPIRV_REFLECT ON CACHE BOOL "" FORCE)
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

add_subdirectory(thirdparty/LLGL)
add_subdirectory(thirdparty/glm)
add_subdirectory(thirdparty/glfw)
add_subdirectory(thirdparty/STB)
add_subdirectory(thirdparty/freetype-2.13.2)
add_subdirectory(thirdparty/tracy-0.11.1)

if(SGE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

file(GLOB_RECURSE SGE_SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.mm")
if (NOT APPLE)
    set_source_files_properties(${SGE_SRC} PROPERTIES COMPILE_FLAGS "-x c++")
endif()

if(SGE_BUILD_STATIC)
    add_library(${PROJECT_NAME} STATIC)
else()
    add_library(${PROJECT_NAME} SHARED)
endif()

target_sources(${PROJECT_NAME} PRIVATE ${SGE_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/include")

if(NOT MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE "-Wall" "-Wextra" "-Wno-gnu-zero-variadic-macro-arguments")
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE "/Zc:preprocessor")
endif()

if(RELEASE_BUILD)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SGE_DEBUG=0)
else()
    target_compile_definitions(${PROJECT_NAME} PUBLIC SGE_DEBUG=1)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC glm STB glfw LLGL freetype TracyClient)

# ========================= Embed Shaders =========================

file(GLOB_RECURSE SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/renderer/shaders/*.*)

foreach(shader ${SHADERS})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/renderer/shaders.hpp
        COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/embed_shaders.py ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_EXECUTABLE_SUFFIX}
        DEPENDS ${shader}
    )
endforeach()

add_custom_target(EmbedShaders ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/renderer/shaders.hpp)
