#pragma once
#include "OpenGLPanel.h"
#include "ModelPart.h"
#include <GL\glew.h>
#include <glm\glm.hpp>

class UVPanel : public OpenGLPanel
{
public:
	UVPanel();
	~UVPanel();
	void Initialize();
	void Reshape(int width, int height);
	void Display();
	void MouseDown(int x, int y, int button);
	void MouseUp(int x, int y, int button);
	void MouseMove(int x, int y);
	void MouseWheel(int x, int y, int delta);

	ModelPart* part;

private:
	struct DrawTextureProgram {
		GLuint program;
		GLuint textureLocation;
	} drawTexture;
	struct DrawColorProgram {
		GLuint program;
		GLuint modelMatrixLocation;
		GLuint viewMatrixLocation;
		GLuint projectionMatrixLocation;
		GLuint colorLocation;
	} drawColor;
	float zoom;
};
