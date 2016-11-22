#pragma once
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <glm\glm.hpp>

struct MyTraits : public OpenMesh::DefaultTraits
{
	VertexAttributes(OpenMesh::Attributes::Normal);
	FaceAttributes(OpenMesh::Attributes::Normal);
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
	void Extrude(float thickness, int divisions);
	void Smooth();

private:
	static OpenMesh::EPropHandleT<double> edgeWeight;
	static OpenMesh::VPropHandleT<double> oneRingArea;

	void UpdateEdgeWeight();
	double MeshArea();
	double face_area(FaceHandle fh);
	double one_ring_area(VertexHandle vh);

	bool hasExtraction;


};