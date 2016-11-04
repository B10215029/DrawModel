#pragma once
//#pragma warning(disable:4996)
#include <GL/glew.h>

// 讀取圖片並建立Texture
GLuint loadTextureFromArray(const unsigned char* data, int width, int height, int format);

// 根據檔案路徑讀取Shader
GLuint loadShader(const char* filePath, GLenum shaderType);

// 根據資源檔ID讀取Shader
GLuint loadShader(const int fileResourceId, GLenum shaderType);

// 根據Shader的檔案路徑讀取Shader並link到一個新Program
GLuint loadProgram(const char* vertexShaderFilePath, const char* fragmentShaderFilePath);

// 根據Shader的資源檔ID讀取Shader並link到一個新Program
GLuint loadProgram(const int vertexShaderResourceId, const int fragmentShaderResourceId);
