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
	void AddPoint(float x, float y);

private:
	GLuint program;
	GLuint colorUniformLocation;
	std::vector<glm::vec3> points;

};
