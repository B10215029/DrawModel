#pragma once
#include "OpenGLPanel.h"
#include "ModelPart.h"
#include <GL\glew.h>
#include <glm\glm.hpp>

class ARAPPanel : public OpenGLPanel
{
public:
	ARAPPanel();
	~ARAPPanel();
	void Initialize();
	void Reshape(int width, int height);
	void Display();
	void MouseDown(int x, int y, int button);
	void MouseUp(int x, int y, int button);
	void MouseMove(int x, int y);
	void MouseWheel(int x, int y, int delta);
	void SetPart(ModelPart* modelPart);

	ModelPart* part;

private:
	struct DrawColorProgram {
		GLuint program;
		GLuint modelMatrixLocation;
		GLuint viewMatrixLocation;
		GLuint projectionMatrixLocation;
		GLuint colorLocation;
	} drawColor;
	float zoom;
	int selectPoint;
	MyMesh mesh;
	std::vector<glm::vec3> facePointSet;
	std::vector<int> controlPoint;
};

