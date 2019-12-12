# CMakeList.txt : CMake project for Triangle, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

# Add source to this project's executable.
add_executable (Triangle "Triangle.cpp" ${RENDERER_SOURCES} ${SNIPPETS_SOURCES} )

find_package(spirv_cross_core CONFIG REQUIRED)
target_link_libraries(Triangle PRIVATE spirv-cross-core)

find_package(spirv_cross_glsl CONFIG REQUIRED)
target_link_libraries(Triangle PRIVATE spirv-cross-glsl)

find_package(spirv_cross_reflect CONFIG REQUIRED)
target_link_libraries(Triangle PRIVATE spirv-cross-reflect)

find_package(spirv_cross_util CONFIG REQUIRED)
target_link_libraries(Triangle PRIVATE spirv-cross-util)

find_library(SHADERC_LIBRARY shaderc_combined)
target_link_libraries(Triangle PRIVATE ${SHADERC_LIBRARY})

