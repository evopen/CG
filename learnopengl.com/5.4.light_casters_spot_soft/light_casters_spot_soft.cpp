#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "../common/shader.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../common/camera.h"

GLFWwindow* window;
glm::vec3 lightPos(5.f, 3.f, -2.f);

uint32_t width = 800;
uint32_t height = 600;

float lastX = width / 2;
float lastY = height / 2;

Camera camera;

glm::vec3 cubePositions[] = {
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(2.0f, 5.0f, -15.0f),
	glm::vec3(-1.5f, -2.2f, -2.5f),
	glm::vec3(-3.8f, -2.0f, -12.3f),
	glm::vec3(2.4f, -0.4f, -3.5f),
	glm::vec3(-1.7f, 3.0f, -7.5f),
	glm::vec3(1.3f, -2.0f, -2.5f),
	glm::vec3(1.5f, 2.0f, -2.5f),
	glm::vec3(1.5f, 0.2f, -1.5f),
	glm::vec3(-1.3f, 1.0f, -1.5f)
};

float vertices[] = {
	// positions          // normals           // texture coords
	-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
	0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
	0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
	0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
	-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

	-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
	0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
	0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
	0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
	-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

	-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
	-0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

	0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
	0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
	0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
	0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
	0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
	0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	-0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

	-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
	0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
	0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
	-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
	-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
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
	if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS)
	{
		glfwMaximizeWindow(window);
	}
	if (glfwGetKey(window, GLFW_KEY_F12) == GLFW_PRESS)
	{
		glfwRestoreWindow(window);
	}

	float cubeSpeed = 5;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		lightPos += cubeSpeed * deltaTime * glm::vec3(0, 0, -1);
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		lightPos -= cubeSpeed * deltaTime * glm::vec3(0, 0, -1);
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		lightPos += cubeSpeed * deltaTime * glm::vec3(1, 0, 0);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		lightPos -= cubeSpeed * deltaTime * glm::vec3(1, 0, 0);
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
	{
		lightPos += cubeSpeed * deltaTime * glm::vec3(0, 1, 0);
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
	{
		lightPos -= cubeSpeed * deltaTime * glm::vec3(0, 1, 0);
	}
}

void framebuffer_size_callback(GLFWwindow* window, int newWidth, int newHeight)
{
	width = newWidth;
	height = newHeight;
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double x, double y)
{
	float offsetX = x - lastX;
	float offsetY = lastY - y;
	lastX = x;
	lastY = y;

	camera.ProcessMouseMovement(offsetX, offsetY);
}

void scroll_callback(GLFWwindow* window, double offsetX, double offsetY)
{
	camera.ProcessMouseScroll(offsetY);
}

void loadTexture(GLuint& texture, const std::string& path, GLenum format)
{
	stbi_set_flip_vertically_on_load(true);
	int width, height, channels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
	if (!data)
	{
		std::cout << "failed to load texture image" << "\n";
	}
	glCreateTextures(GL_TEXTURE_2D, 1, &texture);
	// set the texture wrapping/filtering options (on the currently bound texture object)

	glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureStorage2D(texture, 1, GL_RGB8, width, height);
	glTextureSubImage2D(texture, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
	glGenerateTextureMipmap(texture);
	stbi_image_free(data);
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

	gladLoadGL();

	glViewport(0, 0, 800, 600);
}


int main(int argc, char* argv[])
{
	init();

	GLuint cubeVAO;
	GLuint lightVAO;
	GLuint VBO;

	// generate VBO
	glCreateBuffers(1, &VBO);
	glNamedBufferData(VBO, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// fill cubeVAO
	glCreateVertexArrays(1, &cubeVAO);
	glVertexArrayVertexBuffer(cubeVAO, 0, VBO, 0, sizeof(float) * 8);
	glVertexArrayAttribFormat(cubeVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(cubeVAO, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
	glVertexArrayAttribFormat(cubeVAO, 2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float));
	glEnableVertexArrayAttrib(cubeVAO, 0);
	glEnableVertexArrayAttrib(cubeVAO, 1);
	glEnableVertexArrayAttrib(cubeVAO, 2);
	glVertexArrayAttribBinding(cubeVAO, 0, 0);
	glVertexArrayAttribBinding(cubeVAO, 1, 0);
	glVertexArrayAttribBinding(cubeVAO, 2, 0);

	// fill lightVAO
	glCreateVertexArrays(1, &lightVAO);
	glVertexArrayVertexBuffer(lightVAO, 0, VBO, 0, sizeof(float) * 8);
	glVertexArrayAttribFormat(lightVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(lightVAO, 0);


	Shader cubeShader("cube.vert", "cube.frag");
	Shader lampShader("lamp.vert", "lamp.frag");

	GLuint diffuseTexture;
	GLuint specularTexture;
	loadTexture(diffuseTexture, "../resources/textures/container2.png", GL_RGBA);
	loadTexture(specularTexture, "../resources/textures/container2_specular.png", GL_RGBA);

	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		glClearColor(0.2, 0.0, 0.0, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_DEPTH_TEST);


		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / height, 0.1f, 100.f);
		glm::mat4 model;
		glm::mat4 view = camera.GetViewMatrix();

		cubeShader.use();
		cubeShader.setInt("material.diffuse", 0);
		cubeShader.setInt("material.specular", 1);
		cubeShader.setFloat("light.constant", 1.0f);
		cubeShader.setFloat("light.linear", 0.09f);
		cubeShader.setFloat("light.quadratic", 0.032f);
		glBindTextureUnit(0, diffuseTexture);
		glBindTextureUnit(1, specularTexture);

		cubeShader.setMat4("view", view);
		cubeShader.setMat4("projection", projection);


		cubeShader.setFloat("material.shininess", 0.088 * 2048);
		cubeShader.setVec3("light.position", camera.Position);
		cubeShader.setVec3("light.direction", camera.Front);
		cubeShader.setFloat("light.innerCutOff", glm::cos(glm::radians(12.5)));
		cubeShader.setFloat("light.outerCutOff", glm::cos(glm::radians(20.0)));

		cubeShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
		cubeShader.setVec3("light.diffuse", 2.f, 2.f, 2.f); // darken the light a bit to fit the scene
		cubeShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		cubeShader.setVec3("cameraPos", camera.Position);
		glBindVertexArray(cubeVAO);
		for (unsigned int i = 0; i < 10; i++)
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			float angle = 20.0f * i;
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			cubeShader.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}
