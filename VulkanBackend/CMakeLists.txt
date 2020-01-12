# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

project ("VulkanBackend")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(RENDERER_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/Renderer/VulkanBase.cpp)
set(SNIPPETS_SOURCES 
	${CMAKE_CURRENT_LIST_DIR}/src/snippets/snippets.h
)

include_directories("src/snippets")
include_directories("src/Renderer")
include_directories("src/dhh")

# sudo apt install libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev
# ./vcpkg install glfw3 vulkan-memory-allocator glm catch2 spirv-cross
find_package(Vulkan REQUIRED)
link_libraries(Vulkan::Vulkan)
find_package(glfw3 CONFIG REQUIRED)
link_libraries(glfw)
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
link_libraries(unofficial::vulkan-memory-allocator::vulkan-memory-allocator)
find_package(glm CONFIG REQUIRED)
link_libraries(glm)
find_package(Catch2 CONFIG REQUIRED)
link_libraries(Catch2::Catch2)

find_package(spirv_cross_core CONFIG REQUIRED)
link_libraries(spirv-cross-core)

find_package(spirv_cross_glsl CONFIG REQUIRED)
link_libraries(spirv-cross-glsl)

find_package(spirv_cross_reflect CONFIG REQUIRED)
link_libraries(spirv-cross-reflect)

find_library(SHADERC_LIBRARY shaderc_combined)
link_libraries(${SHADERC_LIBRARY})

# Include sub-projects.
add_subdirectory("src/snippets")
add_subdirectory("src/10.3.asteroids_instanced")
add_subdirectory("src/Triangle")
add_subdirectory("src/NBody")
add_subdirectory("src/particles")

add_executable(Testing "src/TestingGround/test.cpp")
