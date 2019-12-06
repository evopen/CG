# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

project ("VulkanBackend" LANGUAGES CXX)

set(RENDERER_HEADERS
	${CMAKE_CURRENT_LIST_DIR}/Renderer/VulkanBase.h
)

set(RENDERER_SOURCES
	${CMAKE_CURRENT_LIST_DIR}/Renderer/VulkanBase.cpp
)

include_directories(${CMAKE_CURRENT_LIST_DIR}/Renderer)

find_package(Vulkan REQUIRED)
link_libraries(Vulkan::Vulkan)
find_package(glfw3 CONFIG REQUIRED)
link_libraries(glfw)
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
link_libraries(unofficial::vulkan-memory-allocator::vulkan-memory-allocator)

# Include sub-projects.
add_subdirectory ("Triangle")
add_subdirectory("src/10.3.asteroids_instanced")
