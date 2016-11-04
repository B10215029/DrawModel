#include "ShaderUtility.h"

#include <iostream>
#include <fstream>
#include <Windows.h>
#include <GL/glew.h>

GLuint loadTextureFromArray(const unsigned char* data, int width, int height, int format)
{
	GLuint textureHandle;
	glGenTextures(1, &textureHandle);
	glBindTexture(GL_TEXTURE_2D, textureHandle);
	if (format == 3) {//BGR
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_BLUE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glBindTexture(GL_TEXTURE_2D, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
	}
	if (format == 4) {//BGRA
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_BLUE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glBindTexture(GL_TEXTURE_2D, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	return textureHandle;
}

GLuint loadShader(const char* filePath, GLenum shaderType)
{
	GLuint shader = glCreateShader(shaderType);
	GLchar *shaderSource;

	std::ifstream file(filePath, std::ios::binary);
	int fileSize = file.tellg();
	file.seekg(0, std::ios::end);
	fileSize = (int)file.tellg() - fileSize;
	shaderSource = new char[fileSize + 1];
	file.seekg(0, std::ios::beg);
	file.read(shaderSource, fileSize);
	shaderSource[fileSize] = 0;

	//std::cout << shaderSource;
	glShaderSource(shader, 1, &shaderSource, NULL);
	glCompileShader(shader);
	int compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		std::cout << "Error compile shader:" << filePath << std::endl;
		GLint infoLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen > 1)
		{
			char* infoLog = new char[infoLen];
			glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
			std::cout << infoLog << std::endl;
			delete[] infoLog;
		}
		return 0;
	}
	delete[] shaderSource;
	return shader;
}

GLuint loadShader(const int fileResourceId, GLenum shaderType)
{
	GLuint shader = glCreateShader(shaderType);

	HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(fileResourceId), L"Shader");
	GLint dwSize = SizeofResource(NULL, hRes);
	HGLOBAL hData = LoadResource(NULL, hRes);
	GLchar *shaderSource = (GLchar*)LockResource(hData);
	//std::cout << ">>load shader" << fileResourceId << std::endl;
	//std::cout << shaderSource;
	glShaderSource(shader, 1, &shaderSource, &dwSize);
	glCompileShader(shader);
	int compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		std::cout << "Error compile shader:" << fileResourceId << std::endl;
		std::cout << shaderSource << std::endl;
		GLint infoLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen > 1)
		{
			char* infoLog = new char[infoLen];
			glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
			std::cout << infoLog << std::endl;
			delete[] infoLog;
		}
		return 0;
	}
	return shader;
}

GLuint loadProgram(const char* vertexShaderFilePath, const char* fragmentShaderFilePath)
{
	int vertexShader = loadShader(vertexShaderFilePath, GL_VERTEX_SHADER);
	int fragmentShader = loadShader(fragmentShaderFilePath, GL_FRAGMENT_SHADER);
	int program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	int linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked) {
		std::cout << "Error linking program" << std::endl;
		GLint infoLen = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen > 1)
		{
			char* infoLog = new char[infoLen];
			glGetProgramInfoLog(program, infoLen, NULL, infoLog);
			std::cout << infoLog << std::endl;
			delete[] infoLog;
		}
		glDeleteProgram(program);
		return 0;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return program;
}

GLuint loadProgram(const int vertexShaderResourceId, const int fragmentShaderResourceId)
{
	//std::cout << ">>load program " << vertexShaderResourceId << " and " << fragmentShaderResourceId << std::endl;
	int vertexShader = loadShader(vertexShaderResourceId, GL_VERTEX_SHADER);
	int fragmentShader = loadShader(fragmentShaderResourceId, GL_FRAGMENT_SHADER);
	int program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	int linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked) {
		std::cout << "Error linking program" << std::endl;
		GLint infoLen = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen > 1)
		{
			char* infoLog = new char[infoLen];
			glGetProgramInfoLog(program, infoLen, NULL, infoLog);
			std::cout << infoLog << std::endl;
			delete[] infoLog;
		}
		glDeleteProgram(program);
		return 0;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return program;
}
