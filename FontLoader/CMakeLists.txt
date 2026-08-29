cmake_minimum_required(VERSION 3.18)

project(SGE_FONT_LOADER LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(../thirdparty/freetype-2.13.2 ${CMAKE_CURRENT_BINARY_DIR}/thirdparty/freetype-2.13.2)

set(SGE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sdf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vector.cpp
)

add_library(SGE_FontLoader STATIC)
target_sources(SGE_FontLoader PRIVATE ${SGE_SRC})

target_compile_definitions(SGE_FontLoader PUBLIC "$<$<CONFIG:Debug>:SGE_DEBUG>")
target_include_directories(SGE_FontLoader PUBLIC "${PROJECT_SOURCE_DIR}/include")

target_include_directories(SGE_FontLoader PRIVATE "${PROJECT_SOURCE_DIR}/../thirdparty/freetype-2.13.2/include")
target_link_libraries(SGE_FontLoader PRIVATE freetype)

target_link_libraries(SGE_FontLoader PUBLIC glm)