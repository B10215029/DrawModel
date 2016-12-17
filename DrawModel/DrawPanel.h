#pragma once
#include "OpenGLPanel.h"
#include <GL\glew.h>
#include <glm\glm.hpp>
#include <vector>
#include <deque>
#include "ModelPart.h"

class DrawPanel : public OpenGLPanel
{
public:

	enum ViewDirection
	{
		FrontView,
		BackView,
		TopView,
		BottomView,
		RightView,
		LeftView,
		FTRView,
		BBLView,
		ResetView
	};

	enum OperateMode
	{
		PointMode,
		CreateMode,
		DrawMode,
		DeformationMode
	};

	DrawPanel();
	~DrawPanel();
	void Initialize();
	void Reshape(int width, int height);
	void Display();
	void MouseDown(int x, int y, int button);
	void MouseUp(int x, int y, int button);
	void MouseMove(int x, int y);
	void MouseWheel(int x, int y, int delta);
	void RemovePart(int id);
	void ReCreatePart(int id);
	void ExtractionPart(int id, float s);
	void SmoothPart(int id, int step);
	void SetView(ViewDirection viewDirection);
	void ExportOBJ(const char* fileName);

	bool cleanStroke;
	glm::vec3 transform;
	glm::vec3 rotation;
	OperateMode operateMode;
	int selectPart;

private:
	glm::ivec2 previousMousePosition;
	//glm::mat4 modelMatrix;
	//glm::mat4 viewMatrix;
	//glm::mat4 projectionMatrix;
	float zoom, zNear, zFar, zDraw;
	GLuint program;
	GLuint modelMatrixLocation;
	GLuint viewMatrixLocation;
	GLuint projectionMatrixLocation;
	GLuint fboTextureLocation;

	GLuint strokeProgram;
	GLuint strokeTextureLocation;
	GLuint strokeColorLocation;
	GLuint strokeFBO;
	GLuint strokeFBOColorTexture;
	GLuint strokeTextureHandle;
	std::vector<glm::vec3> points;
	std::vector<glm::vec3> pointQueue;
	std::deque<ModelPart*> parts;
};
