#include "MyMesh.h"
#undef min
#undef max
#include <iostream>
#include <ctime>
#include <deque>
#include <Eigen/Sparse>
#include <OpenMesh/Tools/Smoother/JacobiLaplaceSmootherT.hh>

OpenMesh::EPropHandleT<double> MyMesh::edgeWeight;
OpenMesh::VPropHandleT<double> MyMesh::oneRingArea;

MyMesh::MyMesh()
{
	add_property(edgeWeight);
	add_property(oneRingArea);
	hasExtraction = false;
}

MyMesh::~MyMesh()
{

}

MyMesh* MyMesh::CreateFace(std::vector<glm::vec3> contourPoints)
{
	MyMesh* mesh = new MyMesh();

	OpenMesh::IO::Options options;
	if (OpenMesh::IO::read_mesh(*mesh, "./neptune_reduce.obj", options)) {
		if (!options.check(OpenMesh::IO::Options::VertexNormal) && mesh->has_vertex_normals()) {
			mesh->update_normals();
		}
	}
	else {
		std::cerr << "read error\n";
		delete mesh;
	}
	return mesh;
}

void MyMesh::Extraction(float s)
{
	static Eigen::SparseMatrix<double> a;
	static Eigen::MatrixXd b;
	static Eigen::MatrixXd x;
	static Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> linearSolver;
	static double wl;
	clock_t c = clock();
	std::cout << "start:" << c << std::endl;
	int vertexCount = n_vertices();
	UpdateEdgeWeight();
	std::cout << "UpdateEdgeWeight:" << c - clock() << std::endl;
	if (!hasExtraction) {
		hasExtraction = true;
		wl = sqrt(MeshArea() / n_faces()) * 0.001;
		//std::cout << "MeshArea()" << wl << std::endl;
		a = Eigen::SparseMatrix<double>(vertexCount * 2, vertexCount);
		b = Eigen::MatrixXd(vertexCount * 2, 3);
		x = Eigen::MatrixXd(vertexCount, 3);
		std::cout << "wl = sqrt(MeshArea() / n_faces()) * 0.001;:" << c - clock() << std::endl;
		// fill matrix
		for (VertexIter v_it = vertices_begin(); v_it != vertices_end(); ++v_it) {
			double w = 0;
			for (VertexEdgeIter ve_it = ve_begin(v_it); ve_it != ve_end(v_it); ++ve_it) {
				w += property(edgeWeight, ve_it);
			}
			for (VertexOHalfedgeIter voh_it = voh_begin(v_it); voh_it != voh_end(v_it); ++voh_it) {
				double ew = property(edgeWeight, edge_handle(voh_it));
				a.insert(v_it->idx(), to_vertex_handle(voh_it).idx()) = ew * wl / w;
				//w += ew;
			}
			a.insert(v_it->idx(), v_it->idx()) = -wl;
			property(oneRingArea, v_it) = one_ring_area(v_it);
			a.insert(vertexCount + v_it->idx(), v_it->idx()) = 1;
			b(v_it->idx(), 0) = 0;
			b(v_it->idx(), 1) = 0;
			b(v_it->idx(), 2) = 0;
			b(vertexCount + v_it->idx(), 0) = point(v_it)[0];
			b(vertexCount + v_it->idx(), 1) = point(v_it)[1];
			b(vertexCount + v_it->idx(), 2) = point(v_it)[2];
		}
		std::cout << "fill matrix:" << c - clock() << std::endl;
		// slove
		a.makeCompressed();
		std::cout << "makeCompressed:" << c - clock() << std::endl;
		linearSolver.compute(a.transpose() * a);
		std::cout << "compute:" << c - clock() << std::endl;
		x.col(0) = linearSolver.solve(a.transpose() * b.col(0));
		std::cout << "solve0:" << c - clock() << std::endl;
		x.col(1) = linearSolver.solve(a.transpose() * b.col(1));
		std::cout << "solve1:" << c - clock() << std::endl;
		x.col(2) = linearSolver.solve(a.transpose() * b.col(2));
		std::cout << "solve2:" << c - clock() << std::endl;
	}
	else {
		wl *= s;
		//std::cout << "wl:" << wl << std::endl;
		for (VertexIter v_it = vertices_begin(); v_it != vertices_end(); ++v_it) {
			double w = 0;
			for (VertexEdgeIter ve_it = ve_begin(v_it); ve_it != ve_end(v_it); ++ve_it) {
				w += property(edgeWeight, ve_it);
			}
			for (VertexOHalfedgeIter voh_it = voh_begin(v_it); voh_it != voh_end(v_it); ++voh_it) {
				double ew = property(edgeWeight, edge_handle(voh_it));
				a.coeffRef(v_it->idx(), to_vertex_handle(voh_it).idx()) = ew * wl / w;
			}
			a.coeffRef(v_it->idx(), v_it->idx()) = -wl;
			double ora = one_ring_area(v_it);
			double& preOra = property(oneRingArea, v_it);
			double& wh = a.coeffRef(vertexCount + v_it->idx(), v_it->idx());
			wh = sqrt(preOra / ora);
			//preOra = ora;
			b(vertexCount + v_it->idx(), 0) = x(v_it->idx(), 0) * wh;
			b(vertexCount + v_it->idx(), 1) = x(v_it->idx(), 1) * wh;
			b(vertexCount + v_it->idx(), 2) = x(v_it->idx(), 2) * wh;
			//std::cout << "w" << w << "wh:" << wh << ", ora" << ora << "(" << b(vertexCount + v_it->idx(), 0) << "," << b(vertexCount + v_it->idx(), 1) << "," << b(vertexCount + v_it->idx(), 2) << ")" << std::endl;
		}
		std::cout << "fill matrix:" << c - clock() << std::endl;
		a.makeCompressed();
		std::cout << "makeCompressed:" << c - clock() << std::endl;
		linearSolver.compute(a.transpose() * a);
		std::cout << "compute:" << c - clock() << std::endl;
		x.col(0) = linearSolver.solve(a.transpose() * b.col(0));
		std::cout << "solve0:" << c - clock() << std::endl;
		x.col(1) = linearSolver.solve(a.transpose() * b.col(1));
		std::cout << "solve1:" << c - clock() << std::endl;
		x.col(2) = linearSolver.solve(a.transpose() * b.col(2));
		std::cout << "solve2:" << c - clock() << std::endl;
	}
	// fill mesh
	for (VertexIter v_it = vertices_begin(); v_it != vertices_end(); ++v_it) {
		point(v_it)[0] = x(v_it->idx(), 0);
		point(v_it)[1] = x(v_it->idx(), 1);
		point(v_it)[2] = x(v_it->idx(), 2);
	}
	std::cout << "fill mesh:" << c - clock() << std::endl;
	puts("Extraction");
}

void MyMesh::LaplacianSmooth()
{

}

void MyMesh::UpdateEdgeWeight()
{
	for (EdgeIter e_it = edges_begin(); e_it != edges_end(); ++e_it) {
		if (!is_boundary(e_it)) {
			HalfedgeHandle he = halfedge_handle(e_it, 0);
			Point vertex0 = point(from_vertex_handle(he));
			Point vertex1 = point(to_vertex_handle(he));
			Point vertex2 = point(opposite_vh(he));
			Point vertex3 = point(opposite_he_opposite_vh(he));
			Point vectorA1 = vertex0 - vertex2;
			Point vectorA2 = vertex1 - vertex2;
			Point vectorB1 = vertex0 - vertex3;
			Point vectorB2 = vertex1 - vertex3;
			double cotA = 1.0 / tan(acos(dot(vectorA1, vectorA2) / vectorA1.length() / vectorA2.length()));
			double cotB = 1.0 / tan(acos(dot(vectorB1, vectorB2) / vectorB1.length() / vectorB2.length()));
			property(edgeWeight, e_it.handle()) = cotA + cotB;
		}
		else {
			property(edgeWeight, e_it.handle()) = 0;
		}
	}
}

double MyMesh::MeshArea()
{
	double area = 0;
	for (FaceIter f_it = faces_begin(); f_it != faces_end(); ++f_it) {
		area += face_area(f_it);
	}
	return area;
}

double MyMesh::face_area(FaceHandle fh)
{
	FaceVertexIter fv_it = fv_begin(fh);
	Point vertex0 = point(fv_it);
	++fv_it;
	Point vertex1 = point(fv_it);
	++fv_it;
	Point vertex2 = point(fv_it);
	//double a = (vertex0 - vertex1).length();
	//double b = (vertex1 - vertex2).length();
	//double c = (vertex2 - vertex0).length();
	//double s = (a + b + c) / 2;
	//double r = sqrt(s * (s - a) * (s - b) * (s - c));
	Point x = vertex1 - vertex0;
	Point y = vertex2 - vertex0;
	double r = sqrt((x[1]*y[2]-x[2]*y[1])*(x[1] * y[2] - x[2] * y[1])+ (x[2] * y[0] - x[0] * y[2])*(x[2] * y[0] - x[0] * y[2])+ (x[0] * y[1] - x[1] * y[0])*(x[0] * y[1] - x[1] * y[0])) * 0.5;
	//std::cout << r;
	return r;
}

double MyMesh::one_ring_area(VertexHandle vh)
{
	double area = 0;
	for (VertexFaceIter vf_it = vf_begin(vh); vf_it != vf_end(vh); ++vf_it) {
		area += face_area(vf_it);
	}
	return area;
}

void MyMesh::Extrude(float thickness, int divisions, float offsetZ, float swellSize, float swellPower)
{
	int vertexCount = n_vertices();
	int faceCount = n_faces();
	int boundaryVertexCount = 0;
	// clone vertex
	Point extrudeDirection = -normal(vertex_handle(0));
	Point offset = normal(vertex_handle(0)) * (thickness / 2 + offsetZ);
	for (int i = 0; i < vertexCount; i++) {
		point(vertex_handle(i)) += offset;
		add_vertex(point(vertex_handle(i)) + extrudeDirection * thickness);
	}
	// find boundary vertex and add vertex
	std::deque<VertexHandle> boundaryVertices;
	boundaryVertices.clear();
	for (HalfedgeIter he_it = halfedges_begin(); he_it != halfedges_end(); ++he_it) {
		if (is_boundary(he_it)) {
			HalfedgeHandle he = he_it.handle();
			do {
				boundaryVertices.push_back(from_vertex_handle(he));
				he = next_halfedge_handle(he);
			} while (he != he_it.handle());
			boundaryVertexCount = boundaryVertices.size();
			break;
		}
	}
	for (int i = 1; i < divisions; i++) {
		for (int j = 0; j < boundaryVertexCount; j++) {
			boundaryVertices.push_back(add_vertex(point(boundaryVertices[j]) + extrudeDirection * (thickness / divisions * i)));
		}
	}
	for (int i = 0; i < boundaryVertexCount; i++) {
		boundaryVertices.push_back(vertex_handle(boundaryVertices[i].idx() + vertexCount));
	}
	MyMesh::VertexHandle vh[3];
	// add boundary face
	for (int i = 0; i < boundaryVertexCount; i++) {
		int ni = (i + 1) % boundaryVertexCount;
		for (int j = 0; j < divisions; j++) {
			MyMesh::VertexHandle p0 = boundaryVertices[boundaryVertexCount * j + i];
			MyMesh::VertexHandle p1 = boundaryVertices[boundaryVertexCount * j + ni];
			MyMesh::VertexHandle p2 = boundaryVertices[boundaryVertexCount * (j + 1) + i];
			MyMesh::VertexHandle p3 = boundaryVertices[boundaryVertexCount * (j + 1) + ni];
			add_face(p0, p1, p2);
			add_face(p2, p1, p3);
		}
	}
	// clone face
	FaceIter f_it = faces_begin();
	for (int i = 0; i < faceCount; ++i) {
		FaceVertexIter fv_it = fv_begin(f_it);
		vh[0] = vertex_handle(fv_it.handle().idx() + vertexCount);
		++fv_it;
		vh[2] = vertex_handle(fv_it.handle().idx() + vertexCount);
		++fv_it;
		vh[1] = vertex_handle(fv_it.handle().idx() + vertexCount);
		add_face(vh, 3);
		++f_it;
	}
	//size with power

	for (int i = 0; i < vertexCount; i++)
	{
		double minDistance = -1;
		double maxDistance = -1;
		if (is_boundary(vertex_handle(i))) {

		}
		else {
			for (int j = 0; j < boundaryVertexCount; j++)
			{
				point(vertex_handle(i));
				//point(vertex_handle(i)) += extrudeDirection * 2;

				double distance = sqrt(
					pow(point(boundaryVertices[j]).data()[0] - point(vertex_handle(i)).data()[0], 2) +
					pow(point(boundaryVertices[j]).data()[1] - point(vertex_handle(i)).data()[1], 2) +
					pow(point(boundaryVertices[j]).data()[2] - point(vertex_handle(i)).data()[2], 2)
				);
				if (minDistance < 0 || minDistance >= distance) {
					minDistance = distance;
				}
				if (maxDistance < 0 || maxDistance <= distance) {
					maxDistance = distance;
				}
			}

			double moveDistance = pow(minDistance/(maxDistance), swellPower) * swellSize;
			point(vertex_handle(i)) -= extrudeDirection * moveDistance;
			point(vertex_handle(i + vertexCount)) += extrudeDirection * moveDistance;
		}
		
		
	}

	update_normals();
	OpenMesh::IO::write_mesh(*this, "test2.obj");
}

void MyMesh::Smooth(int steps)
{
	OpenMesh::Smoother::JacobiLaplaceSmootherT<MyMesh> smoother(*this);
	smoother.initialize(OpenMesh::Smoother::SmootherT<MyMesh>::Component::Tangential_and_Normal, OpenMesh::Smoother::SmootherT<MyMesh>::Continuity::C0);
	smoother.smooth(steps);// Execute 3 smooth steps
	
}
