#pragma once
#include "OpenGLPanel.h"
#include <GL\glew.h>
#include <glm\glm.hpp>
#include <vector>

class DrawPanel : public OpenGLPanel
{
public:
	DrawPanel();
	~DrawPanel();
	void Initialize();
	void Reshape(int width, int height);
	void Display();
	void MouseDown(int x, int y, int button);
	void MouseUp(int x, int y, int button);
	void MouseMove(int x, int y);
	void MouseWheel(int x, int y, int delta);
	void AddPoint(float x, float y, float z);

private:
	glm::ivec2 previousMousePosition;
	glm::mat4 modelMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	glm::vec3 transform;
	glm::vec3 rotation;
	GLuint program;
	GLuint modelMatrixLocation;
	GLuint viewMatrixLocation;
	GLuint projectionMatrixLocation;
	GLuint colorLocation;
	std::vector<glm::vec3> points;

};
