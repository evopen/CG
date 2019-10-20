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
#include <map>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
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
void createOffscreenFB(GLuint& FBO, GLuint& texColorBuffer, GLuint& rbo);
GLuint loadCubeMap(std::vector<std::filesystem::path> faces, std::filesystem::path directory);

struct PostEffect
{
	bool inversion;
	bool grayScale;
	bool sharpen;
	bool blur;
	bool edgeDetect;
	bool paint;
} postEffect;

int sampleRadius = 3;

std::vector<std::filesystem::path> faces
{
	"right.jpg",
	"left.jpg",
	"top.jpg",
	"bottom.jpg",
	"front.jpg",
	"back.jpg"
};

enum TextureMode
{
	Original,
	Reflection,
	Refraction,
};

TextureMode mode;


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

float cubeVertices[] = {
	// positions          // normals
	-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
	0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
	0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
	0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
	-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
	-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,

	-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
	0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
	0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
	0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
	-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

	-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
	-0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
	-0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
	-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,

	0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
	0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
	0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
	0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
	0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
	0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,

	-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
	0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
	0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
	0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
	-0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
	-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,

	-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
	0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
	0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
	0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
	-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
	-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f
};
float planeVertices[] = {
	// positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
	5.0f, -0.5f, 5.0f, 2.0f, 0.0f,
	-5.0f, -0.5f, 5.0f, 0.0f, 0.0f,
	-5.0f, -0.5f, -5.0f, 0.0f, 2.0f,

	5.0f, -0.5f, 5.0f, 2.0f, 0.0f,
	-5.0f, -0.5f, -5.0f, 0.0f, 2.0f,
	5.0f, -0.5f, -5.0f, 2.0f, 2.0f
};

float transparentVertices[] = {
	// positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
	0.0f, 0.5f, 0.0f, 0.0f, 0.0f,
	0.0f, -0.5f, 0.0f, 0.0f, 1.0f,
	1.0f, -0.5f, 0.0f, 1.0f, 1.0f,

	0.0f, 0.5f, 0.0f, 0.0f, 0.0f,
	1.0f, -0.5f, 0.0f, 1.0f, 1.0f,
	1.0f, 0.5f, 0.0f, 1.0f, 0.0f
};

float quadVertices[] = {
	// vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
	// positions   // texCoords
	-1.0f, 1.0f, 0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f, 0.0f,
	1.0f, -1.0f, 1.0f, 0.0f,

	-1.0f, 1.0f, 0.0f, 1.0f,
	1.0f, -1.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 1.0f, 1.0f
};
float skyboxVertices[] = {
	// positions          
	-1.0f, 1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, 1.0f, -1.0f,
	-1.0f, 1.0f, -1.0f,

	-1.0f, -1.0f, 1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f, 1.0f, -1.0f,
	-1.0f, 1.0f, -1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, -1.0f, 1.0f,

	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, -1.0f, 1.0f,
	-1.0f, -1.0f, 1.0f,

	-1.0f, 1.0f, -1.0f,
	1.0f, 1.0f, -1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, 1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, 1.0f,
	1.0f, -1.0f, 1.0f
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
	glDeleteFramebuffers(1, &FBO);
	glDeleteTextures(1, &texColorBuffer);
	glDeleteBuffers(1, &rbo);
	createOffscreenFB(FBO, texColorBuffer, rbo);
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

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 450");
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

void drawOverlay()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Post Effects");
	ImGui::Checkbox("Inversion", &postEffect.inversion);
	ImGui::Checkbox("Gray Scale", &postEffect.grayScale);
	ImGui::Checkbox("Sharpen", &postEffect.sharpen);
	ImGui::Checkbox("Blur", &postEffect.blur);
	ImGui::Checkbox("Edge Detection", &postEffect.edgeDetect);
	ImGui::Checkbox("Oil Paint", &postEffect.paint);
	ImGui::DragInt("Sample Rad", &sampleRadius, 0.1, 1, 18);

	if (ImGui::TreeNode("Directional Light"))
	{
		ImGui::SliderFloat("Light X Direction", &lightDir.x, -1.0f, 1.0f);
		ImGui::SliderFloat("Light Y Direction", &lightDir.y, -1.0f, 1.0f);
		ImGui::SliderFloat("Light Z Direction", &lightDir.z, -1.0f, 1.0f);
		ImGui::TreePop();
	}
	if (ImGui::RadioButton("Original", mode == Original)) { mode = Original; }
	ImGui::SameLine();
	if (ImGui::RadioButton("Reflection", mode == Reflection)) { mode = Reflection; }
	ImGui::SameLine();
	if (ImGui::RadioButton("Refraction", mode == Refraction)) { mode = Refraction; }
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void run()
{
	init();

	Shader shader("shader.vert", "shader.frag");
	Shader screenShader("screen_shader.vert", "screen_shader.frag");
	Shader skyboxShader("skybox_shader.vert", "skybox_shader.frag");
	Shader modelShader("model.vert", "model.frag");

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDisable(GL_STENCIL_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	GLuint cubeVAO, cubeVBO;
	glCreateVertexArrays(1, &cubeVAO);
	glCreateBuffers(1, &cubeVBO);
	glNamedBufferData(cubeVBO, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glVertexArrayVertexBuffer(cubeVAO, 0, cubeVBO, 0, sizeof(float) * 6);
	glVertexArrayAttribFormat(cubeVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(cubeVAO, 1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3);
	glEnableVertexArrayAttrib(cubeVAO, 0);
	glEnableVertexArrayAttrib(cubeVAO, 1);
	glVertexArrayAttribBinding(cubeVAO, 0, 0);
	glVertexArrayAttribBinding(cubeVAO, 1, 0);

	GLuint floorVAO, floorVBO;
	glCreateVertexArrays(1, &floorVAO);
	glCreateBuffers(1, &floorVBO);
	glNamedBufferData(floorVBO, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glVertexArrayVertexBuffer(floorVAO, 0, floorVBO, 0, sizeof(float) * 5);
	glVertexArrayAttribFormat(floorVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(floorVAO, 1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 3);
	glEnableVertexArrayAttrib(floorVAO, 0);
	glEnableVertexArrayAttrib(floorVAO, 1);
	glVertexArrayAttribBinding(floorVAO, 0, 0);
	glVertexArrayAttribBinding(floorVAO, 1, 0);

	GLuint transparentVAO, transparentVBO;
	glCreateVertexArrays(1, &transparentVAO);
	glCreateBuffers(1, &transparentVBO);
	glNamedBufferData(transparentVBO, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
	glVertexArrayVertexBuffer(transparentVAO, 0, transparentVBO, 0, sizeof(float) * 5);
	glVertexArrayAttribFormat(transparentVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(transparentVAO, 1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 3);
	glEnableVertexArrayAttrib(transparentVAO, 0);
	glEnableVertexArrayAttrib(transparentVAO, 1);
	glVertexArrayAttribBinding(transparentVAO, 0, 0);
	glVertexArrayAttribBinding(transparentVAO, 1, 0);

	GLuint screenVAO, screenVBO;
	glCreateVertexArrays(1, &screenVAO);
	glCreateBuffers(1, &screenVBO);
	glNamedBufferData(screenVBO, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glVertexArrayVertexBuffer(screenVAO, 0, screenVBO, 0, sizeof(float) * 4);
	glVertexArrayAttribFormat(screenVAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(screenVAO, 1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2);
	glEnableVertexArrayAttrib(screenVAO, 0);
	glEnableVertexArrayAttrib(screenVAO, 1);
	glVertexArrayAttribBinding(screenVAO, 0, 0);
	glVertexArrayAttribBinding(screenVAO, 1, 0);

	GLuint skyboxVAO, skyboxVBO;
	glCreateVertexArrays(1, &skyboxVAO);
	glCreateBuffers(1, &skyboxVBO);
	glNamedBufferData(skyboxVBO, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	glVertexArrayVertexBuffer(skyboxVAO, 0, skyboxVBO, 0, sizeof(float) * 3);
	glVertexArrayAttribFormat(skyboxVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(skyboxVAO, 0);
	glVertexArrayAttribBinding(skyboxVAO, 0, 0);

	GLuint cubemapTexture = loadCubeMap(faces, filesystem::getResourcesPath() + "textures/skybox");
	Model myModel(filesystem::getResourcesPath() + "objects/nanosuit/nanosuit.obj");


	std::vector<glm::vec3> windows;
	windows.emplace_back(-0.5f, 0.0f, 0.51f);
	windows.emplace_back(2.5f, 0.0f, 3.51f);
	windows.emplace_back(0.0f, 0.0f, 0.7f);
	windows.emplace_back(-0.3f, 0.0f, -2.3f);
	windows.emplace_back(0.5f, 0.0f, -0.6f);

	shader.use();
	shader.setInt("skybox", 0);
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);
	modelShader.use();
	modelShader.setInt("skybox", 99);

	createOffscreenFB(FBO, texColorBuffer, rbo);

	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glClearColor(0.0, 0.0, 0.0, 1.f);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / height, 0.1f, 100.f);
		glm::mat4 model = glm::mat4(1.f);
		glm::mat4 view = camera.GetViewMatrix();

		glDepthMask(GL_FALSE);
		skyboxShader.use();
		skyboxShader.setMat4("projection", projection);
		skyboxShader.setMat4("view", glm::mat4(glm::mat3(view)));
		glBindVertexArray(skyboxVAO);
		glBindTextureUnit(0, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthMask(GL_TRUE);

		// draw cubes
		shader.use();
		shader.setMat4("model", model);
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
		glBindVertexArray(cubeVAO);
		glBindTextureUnit(0, cubemapTexture);
		model = glm::mat4(1.f);
		model = glm::scale(model, glm::vec3(4, 4, 4));
		shader.setMat4("model", model);
		shader.setVec3("cameraPos", camera.Position);
		shader.setBool("refraction", true);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		model = glm::translate(glm::mat4(1.f), glm::vec3(5, 3, 5));
		model = glm::scale(model, glm::vec3(4, 4, 4));
		shader.setMat4("model", model);
		shader.setBool("refraction", false);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// draw model
		modelShader.use();
		model = glm::translate(glm::mat4(1.f), glm::vec3(-8, -10, -3));
		modelShader.setMat4("model", model);
		modelShader.setMat4("view", view);
		modelShader.setMat4("projection", projection);
		modelShader.setVec3("cameraPos", camera.Position);
		modelShader.setVec3("dirLight.direction", lightDir);
		modelShader.setVec3("dirLight.ambient", 0.2f, 0.2f, 0.2f);
		modelShader.setVec3("dirLight.diffuse", 0.8f, 0.8f, 0.8f); // darken the light a bit to fit the scene
		modelShader.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);
		modelShader.setInt("textureMode", mode);
		glBindTextureUnit(99, cubemapTexture);
		myModel.Draw(modelShader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		screenShader.use();
		screenShader.setInt("screenTexture", 0);
		screenShader.setBool("postEffect.inversion", postEffect.inversion);
		screenShader.setBool("postEffect.grayScale", postEffect.grayScale);
		screenShader.setBool("postEffect.sharpen", postEffect.sharpen);
		screenShader.setBool("postEffect.blur", postEffect.blur);
		screenShader.setBool("postEffect.edgeDetect", postEffect.edgeDetect);
		screenShader.setBool("postEffect.paint", postEffect.paint);
		screenShader.setInt("sample_radius", sampleRadius);
		glDisable(GL_DEPTH_TEST);
		glBindTextureUnit(0, texColorBuffer);
		glBindVertexArray(screenVAO);
		glClearColor(1.0, 1.0, 1.0, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		drawOverlay();

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
