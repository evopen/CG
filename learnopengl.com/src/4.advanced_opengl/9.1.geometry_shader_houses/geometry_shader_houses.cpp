#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <shader.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <camera.h>
#include <model.h>
#include <filesystem.h>
#include <windows.h>

extern "C" {
_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}


GLFWwindow* window;
glm::vec3 lightPos(5.f, 3.f, -2.f);

uint32_t width = 1024;
uint32_t height = 768;

Camera camera;
static glm::vec3 lightDir = glm::vec3(0.1, 0.1, -1);


GLuint FBO, texColorBuffer, rbo;
GLuint loadCubeMap(std::vector<std::filesystem::path> faces, std::filesystem::path directory);

float points[] = {
	-0.5f, 0.5f, 1.0f, 0.0f, 0.0f, // top-left
	0.5f, 0.5f, 0.0f, 1.0f, 0.0f, // top-right
	0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // bottom-right
	-0.5f, -0.5f, 1.0f, 1.0f, 0.0f // bottom-left
};

void processInput(GLFWwindow* window)
{
	static float lastTime = glfwGetTime();
	float currentTime = glfwGetTime();
	float deltaTime = currentTime - lastTime;
	lastTime = currentTime;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(Camera::FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(Camera::BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(Camera::RIGHT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(Camera::LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(Camera::UP, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(Camera::DOWN, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS)
	{
		glfwMaximizeWindow(window);
	}
	if (glfwGetKey(window, GLFW_KEY_F12) == GLFW_PRESS)
	{
		glfwRestoreWindow(window);
	}
}


void framebuffer_size_callback(GLFWwindow* window, int newWidth, int newHeight)
{
	width = newWidth;
	height = newHeight;
	glViewport(0, 0, width, height);
	glDeleteFramebuffers(1, &FBO);
	glDeleteTextures(1, &texColorBuffer);
	glDeleteBuffers(1, &rbo);
	glfwSetCursorPos(window, width / 2, height / 2);
}

void mouse_callback(GLFWwindow* window, double x, double y)
{
	float offsetX = x - width / 2;
	float offsetY = height / 2 - y;

	if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
	{
		camera.ProcessMouseMovement(offsetX, offsetY);
		glfwSetCursorPos(window, width / 2, height / 2);
	}
}

void scroll_callback(GLFWwindow* window, double offsetX, double offsetY)
{
	if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
	{
		camera.ProcessMouseScroll(offsetY);
	}
}

void init()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "numerous", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, width / 2, height / 2);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetWindowPos(window, 500, 200);

	if (!gladLoadGL())
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}
	printf("OpenGL loaded\n");

	printf("Vendor:          %s\n", glGetString(GL_VENDOR));
	printf("Renderer:        %s\n", glGetString(GL_RENDERER));
	printf("Version OpenGL:  %s\n", glGetString(GL_VERSION));
	printf("Version GLSL:    %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	glViewport(0, 0, width, height);
}

void createOffscreenFB(GLuint& FBO, GLuint& texColorBuffer, GLuint& rbo)
{
	glCreateFramebuffers(1, &FBO);

	glCreateTextures(GL_TEXTURE_2D, 1, &texColorBuffer);
	glTextureStorage2D(texColorBuffer, 1, GL_RGB8, width, height);

	glCreateRenderbuffers(1, &rbo);
	glNamedRenderbufferStorage(rbo, GL_DEPTH24_STENCIL8, width, height);


	glNamedFramebufferTexture(FBO, GL_COLOR_ATTACHMENT0, texColorBuffer, 0);
	glNamedFramebufferRenderbuffer(FBO, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if ((glCheckNamedFramebufferStatus(FBO, GL_FRAMEBUFFER) & GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) ==
		GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
	{
		std::cout << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT" << "\n";
	}
	if ((glCheckNamedFramebufferStatus(FBO, GL_FRAMEBUFFER) & GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) ==
		GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
	{
		std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT " << "\n";
	}
}

GLuint loadCubeMap(std::vector<std::filesystem::path> faces, std::filesystem::path directory)
{
	GLuint texture;
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &texture);

	int width, height, nrChannels;
	std::filesystem::path fullPath;
	fullPath = directory;
	fullPath /= faces[0];
	stbi_load(fullPath.string().c_str(), &width, &height, &nrChannels, 0);
	glTextureStorage2D(texture, 1, GL_RGB8, width, height);

	for (size_t i = 0; i < faces.size(); i++)
	{
		fullPath = directory;
		fullPath /= faces[i];
		unsigned char* data = stbi_load(fullPath.string().c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			GLenum format;
			if (nrChannels == 1)
				format = GL_RED;
			else if (nrChannels == 3)
				format = GL_RGB;
			else if (nrChannels == 4)
				format = GL_RGBA;
			glTextureSubImage3D(texture, 0, 0, 0, i, width, height, 1, format, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
	}
	glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return texture;
}


void run()
{
	init();

	Shader shader("shader.vert", "shader.frag", "shader.geom");

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDisable(GL_STENCIL_TEST);

	glEnable(GL_PROGRAM_POINT_SIZE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	GLuint pointVAO, pointVBO;
	glCreateVertexArrays(1, &pointVAO);
	glCreateBuffers(1, &pointVBO);
	glNamedBufferData(pointVBO, sizeof(points), points, GL_STATIC_DRAW);
	glVertexArrayVertexBuffer(pointVAO, 0, pointVBO, 0, sizeof(float) * 5);
	glVertexArrayAttribFormat(pointVAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(pointVAO, 1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 2);
	glEnableVertexArrayAttrib(pointVAO, 0);
	glEnableVertexArrayAttrib(pointVAO, 1);
	glVertexArrayAttribBinding(pointVAO, 0, 0);
	glVertexArrayAttribBinding(pointVAO, 1, 0);


	while (!glfwWindowShouldClose(window))
	{
		processInput(window);
		glClearColor(0.0, 0.0, 0.0, 1.f);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


		shader.use();
		glBindVertexArray(pointVAO);
		glDrawArrays(GL_POINTS, 0, 4);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
}


int main(int argc, char* argv[])
{
	try
	{
		run();
	}
	catch (const std::runtime_error& e)
	{
		std::cout << e.what() << "\n";
	}

	return 0;
}
