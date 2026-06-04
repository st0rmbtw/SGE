cmake_minimum_required(VERSION 3.12)

project(SGE_EXAMPLES)

add_subdirectory(clay_integration)
add_subdirectory(clock)
add_subdirectory(stresstest)

set(LLGL_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE BOOL "" FORCE)