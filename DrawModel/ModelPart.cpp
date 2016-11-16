#include "ModelPart.h"
#include "ShaderUtility.h"
#include <glm\gtc\type_ptr.hpp>

struct ModelPart::DrawTextureProgram ModelPart::drawTexture = {};
struct ModelPart::DrawStrokeProgram ModelPart::drawStroke = {};
struct ModelPart::DrawSolidProgram ModelPart::drawSolid = {};
glm::vec4 ModelPart::strokeColor(0, 0, 1, 1);
float ModelPart::strokeSize(50);
float ModelPart::pointInterval(0.05f);
GLuint ModelPart::strokeTextureHandle;
//glm::mat4 ModelPart::modelMatrix;
//glm::mat4 ModelPart::viewMatrix;
//glm::mat4 ModelPart::projectionMatrix;

void ModelPart::InitProgram()
{
	drawTexture.program = loadProgram("./shader/DrawTexture.vert", "./shader/DrawTexture.frag");
	drawTexture.textureLocation = glGetUniformLocation(drawTexture.program, "image");
	drawStroke.program = loadProgram("./shader/DrawStroke.vert", "./shader/DrawStroke.frag");
	drawStroke.strokeSizeLocation = glGetUniformLocation(drawStroke.program, "strokeSize");
	drawStroke.colorLocation = glGetUniformLocation(drawStroke.program, "color");
	drawStroke.colorTextureLocation = glGetUniformLocation(drawStroke.program, "colorTexture");
	drawStroke.strokeTextureLocation = glGetUniformLocation(drawStroke.program, "strokeTexture");
	drawSolid.program = loadProgram("./shader/DrawSolid.vert", "./shader/DrawSolid.frag");
	drawSolid.modelMatrixLocation = glGetUniformLocation(drawSolid.program, "model_matrix");
	drawSolid.viewMatrixLocation = glGetUniformLocation(drawSolid.program, "view_matrix");
	drawSolid.projectionMatrixLocation = glGetUniformLocation(drawSolid.program, "projection_matrix");
	strokeTextureHandle = loadTextureFromFilePNG("./stroke/stroke1.png");
}

void ModelPart::SetStrokeTextureFromFile(const char* filePath)
{
	ModelPart::strokeTextureHandle = loadTextureFromFilePNG(filePath);
}

ModelPart::ModelPart()
{
}

ModelPart::~ModelPart()
{
}

void ModelPart::Render()
{
	RenderStroke();
}

void ModelPart::RenderStroke()
{
	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if (!pointQueue.empty()) {
		glBindFramebuffer(GL_FRAMEBUFFER, strokeFBO);
		glUseProgram(drawStroke.program);
		glUniform4fv(drawStroke.colorLocation, 1, glm::value_ptr(strokeColor));
		glUniform1f(drawStroke.strokeSizeLocation, strokeSize);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, strokeTextureHandle);
		glUniform1i(drawStroke.strokeTextureLocation, 0);
		glEnableVertexAttribArray(0);
		glm::vec3 *pointTemp = new glm::vec3[pointQueue.size()];
		int i;
		for (i = 0; !pointQueue.empty(); i++) {
			pointTemp[i] = pointQueue.front();
			pointQueue.pop();
		}
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, pointTemp);
		glDrawArrays(GL_POINTS, 0, i);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	glClear(GL_DEPTH_BUFFER_BIT);
	glUseProgram(drawTexture.program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, strokeFBOColorTexture);
	glUniform1i(drawTexture.textureLocation, 0);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void ModelPart::RenderModel()
{

}

void ModelPart::CreateFrameBuffer(int width, int height)
{
	glGenFramebuffers(1, &strokeFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, strokeFBO);
	glGenTextures(1, &strokeFBOColorTexture);
	glBindTexture(GL_TEXTURE_2D, strokeFBOColorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, strokeFBOColorTexture, 0);
	float clrCol[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE, clrCol);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(clrCol[0], clrCol[1], clrCol[2], clrCol[3]);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ModelPart::AddPoint(float x, float y, float z)
{
	AddPoint(glm::vec3(x, y, z));
}

void ModelPart::AddPoint(glm::vec3 point)
{
	if (points.empty()) {
		pointQueue.push(point);
		points.push_back(point);
	}
	else {
		glm::vec3 vector = point - points.back();
		float lineLength = glm::length(vector);
		vector = glm::normalize(vector);
		for (float i = pointInterval; i < lineLength; i += pointInterval) {
			pointQueue.push(points.back() + vector * i);
		}
		if (lineLength > pointInterval) {
			points.push_back(point);
		}
	}
}
