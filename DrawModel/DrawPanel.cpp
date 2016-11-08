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
	cleanStroke = true;
}

DrawPanel::~DrawPanel()
{
}

void DrawPanel::Initialize()
{
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_POINT_SMOOTH);
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	program = loadProgram("./shader/DrawTexture.vert", "./shader/DrawTexture.frag");
	fboTextureLocation = glGetUniformLocation(program, "image");
	//modelMatrixLocation = glGetUniformLocation(program, "model_matrix");
	//viewMatrixLocation = glGetUniformLocation(program, "view_matrix");
	//projectionMatrixLocation = glGetUniformLocation(program, "projection_matrix");
	//colorLocation = glGetUniformLocation(program, "color");
	strokeProgram = loadProgram("./shader/DrawStroke.vert", "./shader/DrawStroke.frag");
	strokeColorLocation = glGetUniformLocation(strokeProgram, "color");
	strokeTextureLocation = glGetUniformLocation(strokeProgram, "image");
	strokeTextureHandle = loadTextureFromFilePNG("./stroke/stroke1.png");
	glGenFramebuffers(1, &strokeFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, strokeFBO);
	glGenTextures(1, &strokeFBOColorTexture);
	glBindTexture(GL_TEXTURE_2D, strokeFBOColorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, strokeFBOColorTexture, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DrawPanel::Reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 10000.f);
}

void DrawPanel::Display()
{

	glBindFramebuffer(GL_FRAMEBUFFER, strokeFBO);
	if (cleanStroke) {
		glClear(GL_COLOR_BUFFER_BIT);
		cleanStroke = false;
	}
	if (pointQueue.size()  > 0 && isLMBDown) {
		glUseProgram(strokeProgram);
		glUniform4f(strokeColorLocation, 1, 0, 0, 1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, strokeTextureHandle);
		glUniform1i(strokeTextureLocation, 0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, &pointQueue[0]);
		//glDrawArrays(GL_LINE_STRIP, 0, points.size());
		glDrawArrays(GL_POINTS, 0, pointQueue.size());
		pointQueue.clear();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, strokeFBOColorTexture);
	glUniform1i(strokeTextureLocation, 0);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
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
		screenPos.z = 0;
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
	float interval = 0.05;
	glm::vec3 p(x, y, z);
	if (points.size() > 0) {
		glm::vec3 v = p - points[points.size() - 1];
		float l = glm::length(v);
		if (l < interval)
			return;
		v = glm::normalize(v);
		for (float i = interval; i < l; i += interval) {
			glm::vec3 ip = v * i;
			pointQueue.push_back(points[points.size() - 1] + ip);
		}
	}
	points.push_back(p);
	pointQueue.push_back(p);
	//printf("Add %f, %f\n", x, y);
}
