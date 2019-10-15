#include "shader.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

struct Texture
{
	uint32_t id;
	std::string type;
	std::string path;
};

class Mesh
{
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices,
	     const std::vector<Texture>& textures)
		: vertices(vertices),
		  indices(indices),
		  textures(textures)
	{
		setupMesh();
	}

	void Draw(Shader shader);
private:
	uint32_t VAO, VBO, EBO;

	void setupMesh();
};

inline void Mesh::Draw(Shader shader)
{
	uint32_t diffuseNr = 1;
	uint32_t specularNr = 1;

	for (uint32_t i = 0; i < textures.size(); i++)
	{
		std::string number;
		if(textures[i].type == "texture_diffuse")
			number = std::to_string(diffuseNr++);
		else if(textures[i].type == "texture_specular")
			number = std::to_string(specularNr++);

		shader.setInt(("material" + textures[i].type + number), i);
		glBindTextureUnit(i, textures[i].id);
	}
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
}

inline void Mesh::setupMesh()
{
	glCreateBuffers(1, &VBO);
	glNamedBufferData(VBO, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	glCreateBuffers(1, &EBO);
	glNamedBufferData(EBO, sizeof(indices) * indices.size(), indices.data(), GL_STATIC_DRAW);

	glCreateVertexArrays(1, &VAO);

	glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(Vertex));
	glVertexArrayElementBuffer(VAO, EBO);

	glEnableVertexArrayAttrib(VAO, 0);
	glEnableVertexArrayAttrib(VAO, 1);
	glEnableVertexArrayAttrib(VAO, 2);

	glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, Position));
	glVertexArrayAttribFormat(VAO, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, Normal));
	glVertexArrayAttribFormat(VAO, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, TexCoords));

	glEnableVertexArrayAttrib(VAO, 0);
	glEnableVertexArrayAttrib(VAO, 1);
	glEnableVertexArrayAttrib(VAO, 2);
}
