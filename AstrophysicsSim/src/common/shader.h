#pragma once
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>

class Shader
{
public:
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath)
	{
		GLuint vertex = compileShader(vertexPath, GL_VERTEX_SHADER);
		GLuint fragment = compileShader(fragmentPath, GL_FRAGMENT_SHADER);

		GLint success;
		char infoLog[512];

		// link shader program
		programID = glCreateProgram();
		glAttachShader(programID, vertex);
		glAttachShader(programID, fragment);
		glLinkProgram(programID);
		// print error
		glGetProgramiv(programID, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(programID, 512, nullptr, infoLog);
			std::cout << "Failed to link shader program: " << infoLog << "\n";
		}
	}

	Shader(const GLchar* vertexPath, const GLchar* fragmentPath, const GLchar* geometryPath)
	{
		GLuint vertex = compileShader(vertexPath, GL_VERTEX_SHADER);
		GLuint geometry = compileShader(geometryPath, GL_GEOMETRY_SHADER);
		GLuint fragment = compileShader(fragmentPath, GL_FRAGMENT_SHADER);

		GLint success;
		char infoLog[512];

		// link shader program
		programID = glCreateProgram();
		glAttachShader(programID, vertex);
		glAttachShader(programID, geometry);
		glAttachShader(programID, fragment);
		glLinkProgram(programID);
		// print error
		glGetProgramiv(programID, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(programID, 512, nullptr, infoLog);
			std::cout << "Failed to link shader program: " << infoLog << "\n";
		}
	}

	void use()
	{
		glUseProgram(programID);
	}

	void setFloat(const std::string& name, float value)
	{
		GLint location = glGetUniformLocation(programID, name.c_str());
		if (location == -1)
		{
			std::cout << "failed to find float location: " << name << "\n";
		}
		glUniform1f(location, value);
	}

	void setDouble(const std::string& name, double value)
	{
		GLint location = glGetUniformLocation(programID, name.c_str());
		if (location == -1)
		{
			std::cout << "failed to find double location: " << name << "\n";
		}
		glUniform1d(location, value);
	}

	void setInt(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(programID, name.c_str());
		if (location == -1)
		{
			std::cout << "failed to find int location: " << name << "\n";
		}
		glUniform1i(location, value);
	}

	void setVec3(const std::string& name, float a, float b, float c)
	{
		GLint location = glGetUniformLocation(programID, name.c_str());
		if (location == -1)
		{
			std::cout << "failed to find vec3 location: " << name << "\n";
		}
		glUniform3f(location, a, b, c);
	}

	void setVec3(const std::string& name, const glm::vec3& vec3)
	{
		GLint location = glGetUniformLocation(programID, name.c_str());
		if (location == -1)
		{
			std::cout << "failed to find uniform location: " << name << "\n";
		}
		glUniform3fv(location, 1, &vec3[0]);
	}

	void setMat4(const std::string& name, const glm::mat4& mat4)
	{
		GLint location = glGetUniformLocation(programID, name.c_str());
		if (location == -1)
		{
			std::cout << "failed to find mat4 location: " << name << "\n";
		}
		glUniformMatrix4fv(location, 1, GL_FALSE, &mat4[0][0]);
	}

	void setBool(const std::string& name, bool status)
	{
		GLint location = glGetUniformLocation(programID, name.c_str());
		if (location == -1)
		{
			std::cout << "failed to find bool location: " << name << "\n";
		}
		glUniform1i(location, status);
	}

	GLuint programID;

private:
	GLuint compileShader(const GLchar* path, GLenum type)
	{
		std::string code;
		std::ifstream file;

		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try
		{
			file.open(path);
			std::stringstream shaderStream;
			shaderStream << file.rdbuf();
			file.close();
			code = shaderStream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "failed to read shader file: " << e.what() << "\n";
			exit(-1);
		}
		const char* shaderCode = code.c_str();

		/// compile
		GLuint shader;
		GLint success;
		char infoLog[512];

		/// vertex shader
		shader = glCreateShader(type);
		glShaderSource(shader, 1, &shaderCode, nullptr);
		glCompileShader(shader);
		// print error
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 512, nullptr, infoLog);
			std::cout << "Failed to compile vertex shader: " << infoLog << "\n";
		}

		return shader;
	}
};
