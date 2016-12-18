#include "ContourPanel.h"
#include "ShaderUtility.h"
#include "resource.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

ContourPanel::ContourPanel()
{
	part = NULL;
	zoom = 1;
}

ContourPanel::~ContourPanel()
{
}

void ContourPanel::Initialize()
{
	glEnable(GL_CULL_FACE);
	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_POINT_SMOOTH);
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
	drawColor.program = loadProgram(IDR_SHADER6, IDR_SHADER5);
	drawColor.modelMatrixLocation = glGetUniformLocation(drawColor.program, "model_matrix");
	drawColor.viewMatrixLocation = glGetUniformLocation(drawColor.program, "view_matrix");
	drawColor.projectionMatrixLocation = glGetUniformLocation(drawColor.program, "projection_matrix");
	drawColor.colorLocation = glGetUniformLocation(drawColor.program, "color");
}

void ContourPanel::Reshape(int width, int height)
{
	glViewport(0, 0, width, height);
}

void ContourPanel::Display()
{
	static const float outline[12] = {
		-1, -1, 0,
		 1, -1, 0,
		 1,  1, 0,
		-1,  1, 0
	};

	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!part)
		return;
	std::vector<glm::vec3> points;
	part->readContourScreenPoint(points);
	if (points.empty())
		return;
	glm::mat4 modelMat;
	modelMat = glm::scale(glm::mat4(1), glm::vec3(1.0 / zoom, 1.0 / zoom, 1));

	glUseProgram(drawColor.program);
	glUniformMatrix4fv(drawColor.modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMat));
	glUniformMatrix4fv(drawColor.viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));
	glUniformMatrix4fv(drawColor.projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));
	glEnableVertexAttribArray(0);

	glLineWidth(2);
	glUniform4fv(drawColor.colorLocation, 1, glm::value_ptr(glm::vec4(1, 0, 0, 1)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, outline);
	glDrawArrays(GL_LINE_LOOP, 0, 4);

	glLineWidth(3);
	glUniform4fv(drawColor.colorLocation, 1, glm::value_ptr(glm::vec4(0, 0, 0, 1)));
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, &points[0]);
	glDrawArrays(GL_LINE_LOOP, 0, points.size());

	glPointSize(5);
	glUniform4fv(drawColor.colorLocation, 1, glm::value_ptr(glm::vec4(0, 0, 1, 1)));
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, &points[0]);
	glDrawArrays(GL_POINTS, 0, points.size());
}

void ContourPanel::MouseDown(int x, int y, int button)
{

}

void ContourPanel::MouseUp(int x, int y, int button)
{

}

void ContourPanel::MouseMove(int x, int y)
{

}

void ContourPanel::MouseWheel(int x, int y, int delta)
{
	if (delta < 0) {
		zoom++;
	}
	else {
		zoom--;
		if (zoom < 1)
			zoom = 1;
	}
}
