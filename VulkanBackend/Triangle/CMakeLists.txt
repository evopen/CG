# CMakeList.txt : CMake project for Triangle, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.15)

# Add source to this project's executable.
add_executable (Triangle "Triangle.cpp" "Triangle.h"
	${RENDERER_SOURCES} ${RENDERER_HEADERS}
)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(Triangle PRIVATE glfw)

message(${PROJECT_SOURCE_DIR})