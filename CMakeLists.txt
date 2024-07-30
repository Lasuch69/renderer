cmake_minimum_required(VERSION 3.11)
project(renderer VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)
find_package(SDL2 REQUIRED)

# compile shaders
execute_process(COMMAND python3 ${PROJECT_SOURCE_DIR}/shader_gen.py "${PROJECT_SOURCE_DIR}/src/rendering/shaders")

file(GLOB_RECURSE SOURCE src/*.cpp)
set(THIRDPARTY thirdparty/cgltf/cgltf.c thirdparty/stb/stb_image.cpp thirdparty/vma/vk_mem_alloc.cpp)

set_source_files_properties(${SOURCE} PROPERTIES COMPILE_FLAGS "-Wall -g")
set_source_files_properties(${THIRDPARTY} PROPERTIES COMPILE_FLAGS "-O3")

add_executable(app ${SOURCE} ${THIRDPARTY})
target_include_directories(app PRIVATE src thirdparty)
target_link_libraries(app PRIVATE Vulkan::Vulkan SDL2::SDL2)
