#pragma once

#include <glm/glm.hpp>
#include <vector>

struct UniformBufferObject
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
};

const std::vector<Vertex> vertexData = {
	{{0.0f, -0.5f, 0.f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, 0.5f, 0.f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f, 0.f}, {0.0f, 0.0f, 1.0f}}
};