#include "UVPanel.h"
#include "ShaderUtility.h"
#include "resource.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

UVPanel::UVPanel()
{
	part = NULL;
	zoom = 1;
}

UVPanel::~UVPanel()
{
}

void UVPanel::Initialize()
{
	glEnable(GL_CULL_FACE);
	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_POINT_SMOOTH);
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
	drawTexture.program = loadProgram(IDR_SHADER3, IDR_SHADER4);
	drawTexture.textureLocation = glGetUniformLocation(drawTexture.program, "image");
	drawTexture.flipYLocation = glGetUniformLocation(drawTexture.program, "flipY");
	drawColor.program = loadProgram(IDR_SHADER6, IDR_SHADER5);
	drawColor.modelMatrixLocation = glGetUniformLocation(drawColor.program, "model_matrix");
	drawColor.viewMatrixLocation = glGetUniformLocation(drawColor.program, "view_matrix");
	drawColor.projectionMatrixLocation = glGetUniformLocation(drawColor.program, "projection_matrix");
	drawColor.colorLocation = glGetUniformLocation(drawColor.program, "color");
}

void UVPanel::Reshape(int width, int height)
{
	glViewport(0, 0, width, height);
}

void UVPanel::Display()
{
	static const float outline[12] = {
		0, 0, 0, 
		1, 0, 0, 
		1, 1, 0, 
		0, 1, 0
	};

	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	if (!part)
		return;
	std::vector<glm::vec3> points;
	part->readUVEdge(points);
	if (points.empty())
		return;
	glm::mat4 modelMat;
	modelMat = glm::translate(glm::mat4(1), glm::vec3(-1.0 / zoom, -1.0 / zoom, 0));
	modelMat = glm::scale(modelMat, glm::vec3(2.0 / zoom, 2.0 / zoom, 1));

	glUseProgram(drawTexture.program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, part->getStrokeFBOTexture());
	glUniform1i(drawTexture.textureLocation, 0);
	glUniform1i(drawTexture.flipYLocation, 1);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glUseProgram(drawColor.program);
	glUniformMatrix4fv(drawColor.modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMat));
	glUniformMatrix4fv(drawColor.viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));
	glUniformMatrix4fv(drawColor.projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));

	glLineWidth(2);
	glUniform4fv(drawColor.colorLocation, 1, glm::value_ptr(glm::vec4(1, 0, 0, 1)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, outline);
	glDrawArrays(GL_LINE_LOOP, 0, 4);

	glLineWidth(1);
	glUniform4fv(drawColor.colorLocation, 1, glm::value_ptr(glm::vec4(0, 0, 0, 1)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, &points[0]);
	glDrawArrays(GL_LINES, 0, points.size());

	glPointSize(3);
	glUniform4fv(drawColor.colorLocation, 1, glm::value_ptr(glm::vec4(0, 1, 0, 1)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 6, GL_FLOAT, GL_FALSE, 0, &points[0]);
	glDrawArrays(GL_POINTS, 0, points.size());
}

void UVPanel::MouseDown(int x, int y, int button)
{

}

void UVPanel::MouseUp(int x, int y, int button)
{

}

void UVPanel::MouseMove(int x, int y)
{

}

void UVPanel::MouseWheel(int x, int y, int delta)
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

