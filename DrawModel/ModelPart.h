#pragma once
#include <GL\glew.h>
#include <glm\glm.hpp>
#include <vector>
#include <queue>
#include "MyMesh.h"

enum ModelState {
	STATE_NONE,
	STATE_DRAWING,
	STATE_PATH,
	STATE_PLANE,
	STATE_COLUMN,
	STATE_MODEL
};

class ModelPart
{
public:
	static void InitProgram();
	static void SetStrokeTextureFromFile(const char* filePath);
	static void SavePartsToOBJ(std::deque<ModelPart*> parts, const char* fileName);

	static glm::vec4 strokeColor;
	static float strokeSize;
	static float strokeInterval;
	static float contourInterval;
	static float pointInterval;
	static GLuint strokeTextureHandle;
	static glm::mat4 modelMatrix;
	static glm::mat4 viewMatrix;
	static glm::mat4 projectionMatrix;
	static float extrudeThickness;
	static float extrudeDivisions;
	static float extrudeOffset;
	static float swellPower;
	static float swellSize;
	static bool modelRenderPoint;
	static bool modelRenderLine;
	static float triAspect;
	static float triSize;

	ModelPart();
	~ModelPart();
	void Render();
	void RenderStroke();
	void RenderLine();
	void RenderPoint();
	void RenderContour();
	void RenderUV();
	void RenderModel();
	void AddPoint(float x, float y, float z);
	void AddPoint(glm::vec3 point);
	void CreateFrameBuffer(int width, int height);
	void CreateMesh();
	void UpdateMeshBuffer();
	void ExtractionMesh(float s);
	void SmoothMesh(int step);

private:
	static struct DrawTextureProgram {
		GLuint program;
		GLuint textureLocation;
	} drawTexture;
	static struct DrawColorProgram {
		GLuint program;
		GLuint modelMatrixLocation;
		GLuint viewMatrixLocation;
		GLuint projectionMatrixLocation;
		GLuint colorLocation;
	} drawColor;
	static struct DrawStrokeProgram {
		GLuint program;
		GLuint strokeSizeLocation;
		GLuint colorLocation;
		GLuint colorTextureLocation;
		GLuint strokeTextureLocation;
		GLuint modelMatrixLocation;
		GLuint viewMatrixLocation;
		GLuint projectionMatrixLocation;
	} drawStroke;
	static struct DrawSolidProgram {
		GLuint program;
		GLuint modelMatrixLocation;
		GLuint viewMatrixLocation;
		GLuint projectionMatrixLocation;
		GLuint colorLocation;
		GLuint textureLocation;
		GLuint textureHandle;
		GLuint useTextureLocation;
	} drawSolid;

	GLuint vao, vbo, ebo;
	int vertexCount, faceCount;
	float maxPointDist;
	ModelState state;
	MyMesh* mesh;
	GLuint strokeFBO;
	GLuint strokeFBOColorTexture;
	glm::mat4 mvp;
	std::vector<glm::vec3> points;
	std::vector<glm::vec3> screenPoints;
	std::queue<glm::vec3> pointQueue;
};
