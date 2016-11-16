#pragma once
#undef min
#undef max
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <Eigen/Sparse>

struct MyTraits : public OpenMesh::DefaultTraits
{
	VertexAttributes(OpenMesh::Attributes::Normal);
	FaceAttributes(OpenMesh::Attributes::Normal);
	typedef OpenMesh::Vec3d Point;
	typedef OpenMesh::Vec3d Normal;
};

public class MyMesh : public OpenMesh::TriMesh_ArrayKernelT<MyTraits>
{
public:
	MyMesh();
	~MyMesh();
	void Extraction(float s = 2);

private:
	static OpenMesh::EPropHandleT<double> edgeWeight;
	static OpenMesh::VPropHandleT<double> oneRingArea;

	void UpdateEdgeWeight();
	double MeshArea();
	double face_area(FaceHandle fh);
	double one_ring_area(VertexHandle vh);

	bool hasExtraction;
};