# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

project ("VulkanBackend" LANGUAGES CXX)

set(RENDERER_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/Renderer/VulkanBase.cpp)
set(SNIPPETS_SOURCES 
	${CMAKE_CURRENT_LIST_DIR}/src/snippets/snippets.h
)

include_directories("src/snippets")
include_directories("src/Renderer")
include_directories("src/dhh")

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

find_library(SHADERC_LIBRARY shaderc_combined)
link_libraries(${SHADERC_LIBRARY})



# Include sub-projects.
add_subdirectory("src/snippets")
add_subdirectory("src/10.3.asteroids_instanced")
add_subdirectory("src/Triangle")

add_executable(Testing "src/TestingGround/test.cpp")

include_guard(DIRECTORY)
