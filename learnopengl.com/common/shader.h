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
		std::string vertexCode;
		std::string fragmentCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try
		{
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			std::stringstream vShaderStream, fShaderStream;

			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();

			vShaderFile.close();
			fShaderFile.close();

			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "failed to read shader file: " << e.what() << "\n";
			exit(-1);
		}
		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragmentCode.c_str();

		/// compile
		GLuint vertex, fragment;
		GLint success;
		char infoLog[512];

		/// vertex shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, nullptr);
		glCompileShader(vertex);
		// print error
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
			std::cout << "Failed to compile vertex shader: " << infoLog << "\n";
		}

		/// fragment shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, nullptr);
		glCompileShader(fragment);
		// print error
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
			std::cout << "Failed to compile fragment shader: " << infoLog << "\n";
		}

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

	void use()
	{
		glUseProgram(programID);
	}

	void setFloat(const std::string& name, float value)
	{
		GLint location = glGetUniformLocation(programID, name.c_str());
		glUniform1f(location, value);
	}

	void setInt(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(programID, name.c_str());
		glUniform1i(location, value);
	}

	void setVec3(const std::string& name, float a, float b, float c)
	{
		GLint location = glGetUniformLocation(programID, name.c_str());
		glUniform3f(location, a, b, c);
	}
	
	void setVec3(const std::string& name, glm::vec3 &vec3)
	{
		GLint location = glGetUniformLocation(programID, name.c_str());
		glUniform3fv(location, 1, &vec3[0]);
	}

	GLuint programID;
};
