#include <GLFW/glfw3.h>

int main(int argc, char* argv[])
{
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(800, 600, "numerous", nullptr, nullptr);
	while(!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}
}
