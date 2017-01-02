#pragma once
//#pragma warning(disable:4996)
#include <GL/glew.h>

// 只讀取圖片的大小
void getTextureSize(const char* filePath, int size[2]);

// 讀取圖片並建立Texture(使用libpng)
GLuint loadTextureFromFilePNG(const char* filePath);

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

unsigned char* writeTextureToArray(GLuint textureID);

void writeTextureToFilePNG(const char* filePath, GLuint textureID);
