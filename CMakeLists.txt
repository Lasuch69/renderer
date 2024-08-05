cmake_minimum_required(VERSION 3.11)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message(FATAL_ERROR "Prevented in-tree build. Please create a build directory outside of the SDL source code and run \"cmake -S ${CMAKE_SOURCE_DIR} -B .\" from there")
endif()


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(renderer VERSION 0.1.0 LANGUAGES C CXX)

find_package(Vulkan REQUIRED)
find_package(SDL2 REQUIRED)

find_program(GLSLC_EXECUTABLE NAMES glslc HINTS ENV VULKAN_SDK PATH_SUFFIXES bin)

if(NOT GLSLC_EXECUTABLE)
    message(FATAL_ERROR "Could not find glslc. Please make sure it is installed and available in your PATH or set VULKAN_SDK environment variable.")
endif()

set(SHADER_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_BUILD_DIR})

function(compile_shader SHADER_SOURCE SHADER_BINARY)
	add_custom_command(
		OUTPUT ${SHADER_BINARY}
		COMMAND ${GLSLC_EXECUTABLE} ${SHADER_SOURCE} -o ${SHADER_BINARY} -O
		DEPENDS ${SHADER_SOURCE}
		COMMENT "Compiling ${SHADER_SOURCE} to SPIR-V"
	)
	set(SHADER_TARGETS ${SHADER_TARGETS} ${SHADER_BINARY} PARENT_SCOPE)
endfunction()

file(GLOB_RECURSE SHADER_SOURCES
	${CMAKE_SOURCE_DIR}/shaders/*.vert
	${CMAKE_SOURCE_DIR}/shaders/*.frag
	${CMAKE_SOURCE_DIR}/shaders/*.comp
)

foreach(SHADER_SOURCE ${SHADER_SOURCES})
	get_filename_component(FILE_NAME ${SHADER_SOURCE} NAME)
	compile_shader(${SHADER_SOURCE} "${SHADER_BUILD_DIR}/${FILE_NAME}.spv")
endforeach()

add_custom_target(compile_shaders ALL DEPENDS ${SHADER_TARGETS})

file(GLOB_RECURSE SOURCE src/*.cpp)
set(THIRDPARTY
	thirdparty/cgltf/cgltf.c
	thirdparty/stb/stb_image.cpp
	thirdparty/vma/vk_mem_alloc.cpp
    thirdparty/meshoptimizer/meshoptimizer.h
    thirdparty/meshoptimizer/allocator.cpp
    thirdparty/meshoptimizer/clusterizer.cpp
    thirdparty/meshoptimizer/indexcodec.cpp
    thirdparty/meshoptimizer/indexgenerator.cpp
    thirdparty/meshoptimizer/overdrawanalyzer.cpp
    thirdparty/meshoptimizer/overdrawoptimizer.cpp
    thirdparty/meshoptimizer/quantization.cpp
    thirdparty/meshoptimizer/simplifier.cpp
    thirdparty/meshoptimizer/spatialorder.cpp
    thirdparty/meshoptimizer/stripifier.cpp
    thirdparty/meshoptimizer/vcacheanalyzer.cpp
    thirdparty/meshoptimizer/vcacheoptimizer.cpp
    thirdparty/meshoptimizer/vertexcodec.cpp
    thirdparty/meshoptimizer/vertexfilter.cpp
    thirdparty/meshoptimizer/vfetchanalyzer.cpp
    thirdparty/meshoptimizer/vfetchoptimizer.cpp
)

set_source_files_properties(${SOURCE} PROPERTIES COMPILE_FLAGS "-Wall")
set_source_files_properties(${THIRDPARTY} PROPERTIES COMPILE_FLAGS "-O3")

add_executable(app ${SOURCE} ${THIRDPARTY})
target_include_directories(app PRIVATE include src thirdparty)
target_link_libraries(app PRIVATE Vulkan::Vulkan SDL2::SDL2)

add_dependencies(app compile_shaders)
