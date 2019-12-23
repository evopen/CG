# CMakeList.txt : CMake project for Triangle, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

set(TARGET_NAME "particles")

# Add source to this project's executable.
add_executable (${TARGET_NAME} "${TARGET_NAME}.cpp" ${RENDERER_SOURCES})
