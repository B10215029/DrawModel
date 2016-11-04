#include "DrawPanel.h"
#include "ShaderUtility.h"
#include <iostream>

DrawPanel::DrawPanel()
{
	points.clear();
}

DrawPanel::~DrawPanel()
{
}

void DrawPanel::Initialize()
{
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POINT_SMOOTH);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	program = loadProgram("./shader/DrawLine.vert", "./shader/DrawLine.frag");
	colorUniformLocation = glGetUniformLocation(program, "color");
}

void DrawPanel::Reshape(int width, int height)
{
	glViewport(0, 0, width, height);

}

void DrawPanel::Display()
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (points.size() > 0) {
		glLineWidth(100);
		glUseProgram(program);
		glUniform4f(colorUniformLocation, 1, 0, 0, 1);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, &points[0]);
		glDrawArrays(GL_LINE_STRIP, 0, points.size());
	}
}

void DrawPanel::MouseDown(int x, int y, int button)
{
	
}

void DrawPanel::MouseUp(int x, int y, int button)
{

}

void DrawPanel::MouseMove(int x, int y)
{
	if (isLMBDown) {
		//printf("%d, %d, %d, %d\n", x, y, width, height);
		//printf("%f, %f\n", ((float)x) / width * 2 - 1, ((float)y) / height * -2 + 1);
		AddPoint(((float)x) / width * 2 - 1, ((float)y) / height * -2 + 1);
	}
}

void DrawPanel::MouseWheel(int x, int y, int delta)
{

}

void DrawPanel::AddPoint(float x, float y)
{
	points.push_back(glm::vec3(x, y, 0));
	//printf("Add %f, %f\n", x, y);
}
