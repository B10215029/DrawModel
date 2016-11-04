#pragma once
//#pragma warning(disable:4996)
#include <GL/glew.h>

// Ū���Ϥ��ëإ�Texture
GLuint loadTextureFromArray(const unsigned char* data, int width, int height, int format);

// �ھ��ɮ׸��|Ū��Shader
GLuint loadShader(const char* filePath, GLenum shaderType);

// �ھڸ귽��IDŪ��Shader
GLuint loadShader(const int fileResourceId, GLenum shaderType);

// �ھ�Shader���ɮ׸��|Ū��Shader��link��@�ӷsProgram
GLuint loadProgram(const char* vertexShaderFilePath, const char* fragmentShaderFilePath);

// �ھ�Shader���귽��IDŪ��Shader��link��@�ӷsProgram
GLuint loadProgram(const int vertexShaderResourceId, const int fragmentShaderResourceId);
