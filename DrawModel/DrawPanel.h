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
	void UpdateNormal(int id);
	void UpdateUV(int id, int center);
	void SetView(ViewDirection viewDirection);
	void ExportOBJ(const char* fileName);
	void LoadModel(const char* fileName);
	void LoadModelTexture(const char* fileName);
	void ClearModelStroke();
	void SavePart(int id, const char* fileName);
	void ReadPart(const char* fileName);

	bool cleanStroke;
	glm::vec3 transform;
	glm::vec3 rotation;
	OperateMode operateMode;
	std::deque<ModelPart*> parts;
	int selectPart;

private:
	glm::ivec2 previousMousePosition;
	float zoom, zNear, zFar, zDraw;
};
