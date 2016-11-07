#include "DrawPanel.h"
#include "ShaderUtility.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

DrawPanel::DrawPanel()
{
	points.clear();
	modelMatrix = glm::mat4(1.0f);
	viewMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::mat4(1.0f);
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
	modelMatrixLocation = glGetUniformLocation(program, "model_matrix");
	viewMatrixLocation = glGetUniformLocation(program, "view_matrix");
	projectionMatrixLocation = glGetUniformLocation(program, "projection_matrix");
	colorLocation = glGetUniformLocation(program, "color");
}

void DrawPanel::Reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100000.f);
}

void DrawPanel::Display()
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (points.size() > 0) {
		glLineWidth(3);
		glUseProgram(program);
		glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
		glUniform4f(colorLocation, 1, 0, 0, 1);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, &points[0]);
		glDrawArrays(GL_LINE_STRIP, 0, points.size());
	}
}

void DrawPanel::MouseDown(int x, int y, int button)
{
	previousMousePosition.x = x;
	previousMousePosition.y = y;
}

void DrawPanel::MouseUp(int x, int y, int button)
{

}

void DrawPanel::MouseMove(int x, int y)
{
	if (isLMBDown) {
		glm::vec3 screenPos(x, height -y, 0);
		Bind();
		glReadBuffer(GL_FRONT);
		glReadPixels(screenPos.x, screenPos.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &screenPos.z);
		Release();
		glm::vec3 worldPos = glm::unProject(screenPos, modelMatrix, projectionMatrix, glm::vec4(0, 0, width, height));
		AddPoint(worldPos.x, worldPos.y, worldPos.z);
	}
	if (isRMBDown) {
		//glm::mat4 model = glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1, 0, 0));
		//model = glm::rotate(model, rotation.y, glm::vec3(0, 1, 0));
		//glm::mat4 view = glm::translate(glm::mat4(1.0f), transform);
		rotation.x += glm::radians((float)(y - previousMousePosition.y));
		rotation.y += glm::radians((float)(x - previousMousePosition.x));
		modelMatrix = glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1, 0, 0));
		modelMatrix = glm::rotate(modelMatrix, rotation.y, glm::vec3(0, 1, 0));
	}
	previousMousePosition.x = x;
	previousMousePosition.y = y;
}

void DrawPanel::MouseWheel(int x, int y, int delta)
{

}

void DrawPanel::AddPoint(float x, float y, float z)
{
	points.push_back(glm::vec3(x, y, z));
	//printf("Add %f, %f\n", x, y);
}
