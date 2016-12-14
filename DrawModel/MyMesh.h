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
	void ComputeUV(VertexHandle vh = VertexHandle(200), OpenMesh::Vec2f centerUV = OpenMesh::Vec2f(0.5, 0.5));

private:
	static OpenMesh::EPropHandleT<double> edgeWeight;
	static OpenMesh::VPropHandleT<double> oneRingArea;
	static OpenMesh::VPropHandleT<bool> hasUV;

	void UpdateEdgeWeight();
	double MeshArea();
	double face_area(FaceHandle fh);
	double one_ring_area(VertexHandle vh);
	glm::dvec3 MyMesh::PointToVec3(MyMesh::Point p);

	bool hasExtraction;

};