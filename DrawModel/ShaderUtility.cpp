#include "ShaderUtility.h"

#include <iostream>
#include <fstream>
#include <Windows.h>
#include <GL/glew.h>
#include <libpng/png.h>

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
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_BLUE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glBindTexture(GL_TEXTURE_2D, 0);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
	}
	if (format == 4) {//BGRA
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_BLUE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glBindTexture(GL_TEXTURE_2D, 0);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
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
		std::cout << "Error linking program, " << vertexShaderFilePath << " and " << fragmentShaderFilePath << std::endl;
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

void getTextureSize(const char* filePath, int size[2]) {
	png_structp png_ptr;
	png_infop info_ptr;

	unsigned char header[8];    // 8 is the maximum size that can be checked
	FILE *fp = fopen(filePath, "rb"); // open file and test for it being a png
	if (!fp) printf("[read_png_file] File %s could not be opened for reading\n", filePath);
	fread(header, 1, 8, fp);
	if (png_sig_cmp(header, 0, 8)) printf("[read_png_file] File %s is not recognized as a PNG file\n", filePath);

	/* initialize stuff */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) printf("[read_png_file] png_create_read_struct failed\n");
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) printf("[read_png_file] png_create_info_struct failed\n");
	if (setjmp(png_jmpbuf(png_ptr))) printf("[read_png_file] Error during init_io\n");

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	size[0] = png_get_image_width(png_ptr, info_ptr);
	size[1] = png_get_image_height(png_ptr, info_ptr);

	fclose(fp);
}

GLuint loadTextureFromFilePNG(const char* filePath)
{
	int width, height;
	png_byte color_type;
	png_byte bit_depth;

	png_structp png_ptr;
	png_infop info_ptr;
	int number_of_passes;
	png_bytep * row_pointers;

	unsigned char header[8];    // 8 is the maximum size that can be checked
	FILE *fp = fopen(filePath, "rb"); // open file and test for it being a png
	if (!fp) printf("[read_png_file] File %s could not be opened for reading\n", filePath);
	fread(header, 1, 8, fp);
	if (png_sig_cmp(header, 0, 8)) printf("[read_png_file] File %s is not recognized as a PNG file\n", filePath);

	/* initialize stuff */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) printf("[read_png_file] png_create_read_struct failed\n");
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) printf("[read_png_file] png_create_info_struct failed\n");
	if (setjmp(png_jmpbuf(png_ptr))) printf("[read_png_file] Error during init_io\n");

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	number_of_passes = png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);

	/* read file */
	if (setjmp(png_jmpbuf(png_ptr))) printf("[read_png_file] Error during read_image\n");
	row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
	row_pointers[0] = (png_byte*)malloc(png_get_rowbytes(png_ptr, info_ptr) * sizeof(png_bytep) * height);
	for (int y = 0; y<height; y++)
		row_pointers[y] = row_pointers[0] + png_get_rowbytes(png_ptr, info_ptr) / sizeof(png_byte) * y;
	png_read_image(png_ptr, row_pointers);
	fclose(fp);

	GLuint textureHandle;
	glGenTextures(1, &textureHandle);
	glBindTexture(GL_TEXTURE_2D, textureHandle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, *row_pointers);
	if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGBA)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, *row_pointers);
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureHandle;
}

unsigned char* writeTextureToArray(GLuint textureID)
{
	int width, height;
	GLint textureFormat;
	unsigned char* data;
	glBindTexture(GL_TEXTURE_2D, textureID);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &textureFormat);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

	data = (unsigned char*)malloc(sizeof(unsigned char) * (textureFormat == GL_RGB8 ? 3 : 4) * width * height);
	glGetTexImage(GL_TEXTURE_2D, 0, (textureFormat == GL_RGB8 ? GL_RGB : GL_RGBA), GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);
	return data;
}

void writeTextureToFilePNG(const char* filePath, GLuint textureID)
{
	int width, height;
	png_byte color_type = PNG_COLOR_TYPE_RGBA;
	png_byte bit_depth = 8;

	png_structp png_ptr;
	png_infop info_ptr;
	int number_of_passes = 1;
	png_bytep * row_pointers;

	GLint textureFormat;

	glBindTexture(GL_TEXTURE_2D, textureID);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &textureFormat);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

	color_type = (textureFormat == GL_RGB8 ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGBA);
	row_pointers = (png_bytep*)malloc(sizeof(unsigned char*) * height);
	row_pointers[height - 1] = (png_bytep)malloc(sizeof(unsigned char) * (textureFormat == GL_RGB8 ? 3 : 4) * width * height);
	glGetTexImage(GL_TEXTURE_2D, 0, (textureFormat == GL_RGB8 ? GL_RGB : GL_RGBA), GL_UNSIGNED_BYTE, row_pointers[height - 1]);
	glBindTexture(GL_TEXTURE_2D, 0);
	for (int y = 0; y<height; y++)
		row_pointers[height - 1 - y] = row_pointers[height - 1] + (y * (textureFormat == GL_RGB8 ? 3 : 4) * width);

	/* create file */
	FILE *fp = fopen(filePath, "wb");
	if (!fp)
		printf("[write_png_file] File %s could not be opened for writing", filePath);


	/* initialize stuff */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr)
		printf("[write_png_file] png_create_write_struct failed");

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		printf("[write_png_file] png_create_info_struct failed");

	if (setjmp(png_jmpbuf(png_ptr)))
		printf("[write_png_file] Error during init_io");

	png_init_io(png_ptr, fp);


	/* write header */
	if (setjmp(png_jmpbuf(png_ptr)))
		printf("[write_png_file] Error during writing header");

	png_set_IHDR(png_ptr, info_ptr, width, height,
		bit_depth, color_type, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);


	/* write bytes */
	if (setjmp(png_jmpbuf(png_ptr)))
		printf("[write_png_file] Error during writing bytes");

	png_write_image(png_ptr, row_pointers);


	/* end write */
	if (setjmp(png_jmpbuf(png_ptr)))
		printf("[write_png_file] Error during end of write");

	png_write_end(png_ptr, NULL);

	/* cleanup heap allocation */
	free(row_pointers[height - 1]);
	free(row_pointers);

	fclose(fp);
}
