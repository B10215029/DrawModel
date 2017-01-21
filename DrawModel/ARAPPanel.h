#pragma once
#include "OpenGLPanel.h"
#include "ModelPart.h"
#include <GL\glew.h>
#include <glm\glm.hpp>
#include <Eigen/Sparse>

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
	void SelectPoint(int id);
	void UnselectPoint(int id);
	void MovePoint(int id, glm::vec2 newPos);
	void ApplyToPart();
	void ReadAnimation(const char* fileName);
	void StartOutputAnimation(const char* fileName);
	void EndOutputAnimation();

	ModelPart* part;

private:
	void GetGeachTri(MyMesh::FaceHandle fh, Eigen::Matrix<double, 6, 6> &G);
	void GetFeachTri(MyMesh::FaceHandle fh, double K[][4], double F[][4]);
	void GetHeachTri(MyMesh::FaceHandle fh, double H[][6]);
	void PreComputeG();
	void PreComputeF();
	void PreComputeH();
	void PreStep1();
	void PreStep2();
	void Step1();
	void Step2();
	void Deformation();
	Eigen::SparseMatrix<double> BigG, G00, G01, G10, G11, Gprime, B;
	Eigen::SparseMatrix<double> BigH, H00, H01, H10, H11, Hprime, D;
	Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> linearSolver;
	//vector<double**> K, F, invF;

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
	std::vector<bool> flags;
	std::ifstream animationInputStream;
	std::ofstream animationOutputStream;
	bool animating;
};

