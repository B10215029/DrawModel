#pragma once
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <glm\glm.hpp>

struct MyTraits : public OpenMesh::DefaultTraits
{
	VertexAttributes(OpenMesh::Attributes::Normal | OpenMesh::Attributes::TexCoord2D);
	FaceAttributes(OpenMesh::Attributes::Normal);
	//HalfedgeAttributes(OpenMesh::Attributes::TexCoord2D);
	//VertexAttributes(OpenMesh::Attributes::Status | OpenMesh::Attributes::Normal | OpenMesh::Attributes::TexCoord2D);
	//FaceAttributes(OpenMesh::Attributes::Status | OpenMesh::Attributes::Normal);
	//EdgeAttributes(OpenMesh::Attributes::Status);
	HalfedgeAttributes(OpenMesh::Attributes::TexCoord2D);
	typedef OpenMesh::Vec3d Point;
	typedef OpenMesh::Vec3d Normal;
	typedef OpenMesh::Vec2d  TexCoord2D;
};

class MyMesh : public OpenMesh::TriMesh_ArrayKernelT<MyTraits>
{
public:
	static MyMesh* CreateFace(std::vector<glm::vec3> ContourPoints);
	MyMesh();
	~MyMesh();
	void Extraction(float s = 2);
	void LaplacianSmooth();
	void Extrude(float thickness, int divisions, float offsetZ = 0.0f, float swellSize = 0.0f, float swellPower = 1.0f);
	void Smooth(int steps = 3);
	void ResetUV();
	void ResetVertexUV();
	void ComputeUV(VertexHandle vh = VertexHandle(-1), OpenMesh::Vec2d centerUV = OpenMesh::Vec2d(0.5, 0.5));

private:
	static OpenMesh::EPropHandleT<double> edgeWeight;
	static OpenMesh::VPropHandleT<double> oneRingArea;
	static OpenMesh::VPropHandleT<int> ringLevel;
	static OpenMesh::VPropHandleT<int> parentCount;
	static OpenMesh::VPropHandleT<OpenMesh::Vec2d> UVSum;
	static OpenMesh::FPropHandleT<int> UVGroup;

	void UpdateEdgeWeight();
	double MeshArea();
	double face_area(FaceHandle fh);
	double one_ring_area(VertexHandle vh);
	glm::dvec3 MyMesh::PointToVec3(MyMesh::Point p);

	bool hasExtraction;

};