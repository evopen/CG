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

const bool enableOverlay = false;


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


float quadVertices[] = {
	// positions     // colors
	-0.05f, 0.05f, 1.0f, 0.0f, 0.0f,
	0.05f, -0.05f, 0.0f, 1.0f, 0.0f,
	-0.05f, -0.05f, 0.0f, 0.0f, 1.0f,

	-0.05f, 0.05f, 1.0f, 0.0f, 0.0f,
	0.05f, -0.05f, 0.0f, 1.0f, 0.0f,
	0.05f, 0.05f, 0.0f, 1.0f, 1.0f
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

	if (enableOverlay)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 450");
	}
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

void setupState()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDisable(GL_STENCIL_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glClearColor(0.0, 0.0, 0.0, 1.f);
	glDisable(GL_CULL_FACE);
}

glm::dvec2 earthPos = glm::dvec2(0.f, 149597870700.f);
glm::dvec2 moonPos = glm::dvec2(0.f, 385000.f);
glm::dvec2 speed = glm::dvec2(29800.f, 0);
const double earthMass = 5.972 * pow(10, 24);
const double sunMass = 1.988435 * pow(10, 30);
const double moonMass = 7.3459 * pow(10, 22);
const double G = 6.674 * pow(10, -11);
const double scale = 1 / 300000000000.f;

static uint32_t count = 0 ;

glm::dvec2 calGravityForce(double m1, double m2)
{


	
	double r = glm::length(earthPos);
	double f = (G * m1 * m2) / (r * r);
	double cos0 = glm::dot(earthPos, glm::dvec2(1, 0)) / (glm::length(earthPos));
	
	double x = cos0 * f;
	double y = sqrt(f * f - x * x);
	if (earthPos.y < 0.f)
	{
		y = -y;
	}
	return glm::dvec2(x, y);
}

void gravityIteration(double deltaTime)
{
	glm::dvec2 f = calGravityForce(earthMass, sunMass);
	glm::dvec2 dp = deltaTime * f;

	glm::dvec2 dv = dp / earthMass;


	speed -= dv;

	earthPos += speed * deltaTime;
	// std::cout << earthPos.x << " " << earthPos.y << " " << speed.x << " " << speed.y << "\n";
}

glm::dmat4 modelFromPos()
{
	glm::dvec2 scaled = earthPos * scale;
	return glm::translate(glm::dmat4(1.f), glm::dvec3(scaled, 0));
}

void run()
{
	init();
	setupState();

	Shader shader("shader.vert", "shader.frag");

	GLuint uboBuffer;
	glCreateBuffers(1, &uboBuffer);
	glNamedBufferData(uboBuffer, sizeof(glm::mat4) * 2, nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboBuffer);

	float point[] = {
		0.f, 0.f
	};

	GLuint VAO;
	GLuint VBO;
	glCreateVertexArrays(1, &VAO);
	glCreateBuffers(1, &VBO);
	glNamedBufferData(VBO, sizeof(float) * 2, point, GL_STATIC_DRAW);
	glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(float) * 2);
	glVertexArrayAttribFormat(VAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(VAO, 0);
	glVertexArrayAttribBinding(VAO, 0, 0);
	glBindVertexArray(VAO);

	glEnable(GL_PROGRAM_POINT_SIZE);

	float lastTime = glfwGetTime();

	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / height, 0.1f, 1000.f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model = glm::mat4(1.f);
		glNamedBufferSubData(uboBuffer, 0, sizeof(glm::mat4), &projection);
		glNamedBufferSubData(uboBuffer, sizeof(glm::mat4), sizeof(glm::mat4), &view);

		shader.use();
		shader.setMat4("model", model);
		glDrawArrays(GL_POINTS, 0, 1);

		float currentTime = glfwGetTime();
		float deltaTime = currentTime - lastTime;
		gravityIteration(deltaTime * 31536000);
		lastTime = currentTime;

		model = modelFromPos();

		shader.setMat4("model", model);
		glDrawArrays(GL_POINTS, 0, 1);

		if (enableOverlay)
		{
			drawOverlay();
		}

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
