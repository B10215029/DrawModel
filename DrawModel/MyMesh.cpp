#include "MyMesh.h"
#undef min
#undef max
#include <iostream>
#include <ctime>
#include <deque>
#include <Eigen/Sparse>
#include <OpenMesh/Tools/Smoother/JacobiLaplaceSmootherT.hh>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

OpenMesh::EPropHandleT<double> MyMesh::edgeWeight;
OpenMesh::VPropHandleT<double> MyMesh::oneRingArea;
OpenMesh::VPropHandleT<int> MyMesh::ringLevel;
OpenMesh::VPropHandleT<int> MyMesh::parentCount;
OpenMesh::VPropHandleT<OpenMesh::Vec2d> MyMesh::UVSum;

MyMesh::MyMesh()
{
	add_property(edgeWeight);
	add_property(oneRingArea);
	add_property(UVSum);
	add_property(ringLevel);
	add_property(parentCount);
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
	for (VertexIter v_it = vertices_begin(); v_it != vertices_end(); ++v_it) {
		printf("%d: %f, %f\n", v_it->idx(), texcoord2D(v_it).data()[0], texcoord2D(v_it).data()[1]);
	}
}

void MyMesh::ResetUV()
{
	OpenMesh::Vec2d zero(0, 0);
	for (VertexIter v_it = vertices_begin(); v_it != vertices_end(); ++v_it) {
		property(ringLevel, v_it) = -1;
		property(parentCount, v_it) = 0;
		property(UVSum, v_it) = zero;
		set_texcoord2D(v_it, zero);
	}
}

glm::dvec3 MyMesh::PointToVec3(MyMesh::Point p) {
	static double* d;
	d = p.data();
	return glm::dvec3(d[0], d[1], d[2]);
}

void MyMesh::ComputeUV(VertexHandle vh, OpenMesh::Vec2d centerUV)
{
	glm::dvec3 p = PointToVec3(point(vh));
	std::deque<VertexHandle> queue;
	glm::dvec3 np = PointToVec3(normal(vh));
	glm::dvec3 xp = glm::normalize(glm::cross(glm::dvec3(0, 1, 0), np));
	glm::dvec3 yp = glm::normalize(glm::cross(np, xp));
	set_texcoord2D(vh, centerUV);
	property(ringLevel, vh) = 0;
	queue.push_back(vh);
	int i = 0;
	while (!queue.empty()) {
		VertexHandle currentVh = queue.front();
		queue.pop_front();
		//printf("%d: %lf, %lf\n", i++, texcoord2D(currentVh).data()[0], texcoord2D(currentVh).data()[1]);
		glm::dvec3 nr = PointToVec3(normal(currentVh));
		glm::dvec3 xr = glm::normalize(glm::cross(glm::dvec3(0, 1, 0), nr));
		glm::dvec3 yr = glm::normalize(glm::cross(nr, xr));

		double na = glm::angle(np, nr); // normal angle
		//printf("na: %lf, %lf\n", glm::length(glm::rotate(nr, na, glm::cross(np, nr)) - np), glm::length(glm::rotate(nr, na, glm::cross(nr, np)) - np));
		//if (glm::length(glm::rotate(nr, na, glm::cross(np, nr)) - np) > 0.000000001) {
		//	printf("");
		//}
		glm::dvec3 xrp = glm::rotate(xr, na, glm::cross(nr, np));
		double theta = glm::angle(xrp, xp);
		//printf("theta: %lf, %lf, %d\n", glm::length(glm::rotate(xrp, theta, np) - xp), glm::length(glm::rotate(xrp, theta, -np) - xp), glm::dot(glm::cross(xrp, xp), np) > 0);
		//if (glm::length(glm::rotate(xrp, theta, np) - xp) > 0.000000001) {
		//	printf("");
		//}
		if (glm::dot(glm::cross(xrp, xp), np) < 0)
			theta *= -1;
		if (abs(glm::dot(np, nr) - 1) < 0.00001)
			theta = 0;
		//printf("na test: %lf, theta test: %lf, theta: %lf\n", glm::length(glm::rotate(nr, na, glm::cross(nr, np)) - np), glm::length((glm::rotate(xrp, theta, np)) - xp), theta);
		//printf("rl: %lf, theta: %lf\n", rl, theta);
		//printf("rl: %lf, theta: %lf\n", glm::degrees(na), glm::degrees(theta));

		for (VertexVertexIter vv_it = vv_begin(currentVh); vv_it != vv_end(currentVh); ++vv_it) {
			if (property(ringLevel, vv_it) != -1 && property(ringLevel, vv_it) < property(ringLevel, currentVh))
				continue;
			//glm::dvec3 p1 = PointToVec3(point(vv_it));
			//glm::dvec3 p2 = PointToVec3(point(currentVh));
			////if (glm::length(p1 - p) - glm::length(p2 - p) < 0.03)
			////	continue;
			//if (p1.y == p2.y)
			//	continue;
			glm::dvec3 v = PointToVec3(point(vv_it) - point(currentVh));
			glm::dvec2 uv(glm::dot(v, xr), glm::dot(v, yr));
			uv = glm::normalize(uv) * glm::length(v);
			uv = glm::rotate(uv, -theta);
			//OpenMesh::Vec2f texV = (texcoord2D(vv_it) * property(parentCount, vv_it) + OpenMesh::Vec2f(uv.x, uv.y) + texcoord2D(currentVh)) / (property(parentCount, vv_it) + 1);
			property(UVSum, vv_it) += OpenMesh::Vec2f(uv.x, uv.y) + texcoord2D(currentVh);
			property(parentCount, vv_it)++;
			if (property(ringLevel, vv_it) == -1) {
				queue.push_back(vv_it);
				property(ringLevel, vv_it) = property(ringLevel, currentVh) + 1;
			}
			set_texcoord2D(vv_it, property(UVSum, vv_it) / property(parentCount, vv_it));
			//printf("ID: %03d, %03d, UV: %lf, %lf, %lf, LEVEL: %d, %d\n", currentVh.idx(), vv_it->idx(), (texV - OpenMesh::Vec2f(0.5, 0.5)).norm(), texV.data()[0], texV.data()[1], property(ringLevel, vv_it), property(parentCount, vv_it));
		}
	}

	//const double* d = normal(vh).data();
	//glm::vec3 normal(d[0], d[1], d[2]);
	//glm::vec3 base1 = glm::normalize(glm::cross(normal, glm::vec3(1, 0, 0)));
	//glm::vec3 base2 = glm::normalize(glm::cross(normal, base1));
	//set_texcoord2D(vh, centerUV);
	//property(hasUV, vh) = true;
	//for (VertexVertexIter vv_it = vv_begin(vh); vv_it != vv_end(vh); ++vv_it) {
	//	if (property(hasUV, vv_it))
	//		continue;
	//	d = (point(vv_it) - point(vh)).data();
	//	glm::vec3 v(d[0], d[1], d[2]);
	//	glm::vec2 uv(glm::dot(v, base1), glm::dot(v, base2));
	//	uv = glm::normalize(uv) * glm::length(v);
	//	OpenMesh::Vec2f texV = OpenMesh::Vec2f(uv.x, uv.y) * 10 + centerUV;
	//	ComputeUV(vv_it.handle(), OpenMesh::Vec2f(uv.x, uv.y));
	//	//set_texcoord2D(vv_it, texV);
	//}

	//for (VertexIter v_it = vertices_begin(); v_it != vertices_end(); ++v_it) {
	//	set_texcoord2D(v_it, OpenMesh::Vec2f((float)rand() / RAND_MAX, (float)rand() / RAND_MAX));
	//}
	//for (HalfedgeIter he_it = halfedges_begin(); he_it != halfedges_end(); ++he_it) {
	//	set_texcoord2D(he_it, OpenMesh::Vec2f((float)rand() / RAND_MAX, (float)rand() / RAND_MAX));
	//}
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

	double maxDistance = 0;
	for (int i = 0; i < vertexCount; i++)
	{
		if (!is_boundary(vertex_handle(i))) {
			double minDistance = FLT_MAX;
			for (int j = 0; j < boundaryVertexCount; j++)
			{
				double distance = sqrt(
					pow(point(boundaryVertices[j]).data()[0] - point(vertex_handle(i)).data()[0], 2) +
					pow(point(boundaryVertices[j]).data()[1] - point(vertex_handle(i)).data()[1], 2) +
					pow(point(boundaryVertices[j]).data()[2] - point(vertex_handle(i)).data()[2], 2)
				);
				if (minDistance > distance) {
					minDistance = distance;
				}
			}
			if (maxDistance < minDistance) {
				maxDistance = minDistance;
			}
		}
	}
	for (int i = 0; i < vertexCount; i++)
	{
		if (!is_boundary(vertex_handle(i))) {
			double minDistance = FLT_MAX;
			for (int j = 0; j < boundaryVertexCount; j++)
			{
				double distance = sqrt(
					pow(point(boundaryVertices[j]).data()[0] - point(vertex_handle(i)).data()[0], 2) +
					pow(point(boundaryVertices[j]).data()[1] - point(vertex_handle(i)).data()[1], 2) +
					pow(point(boundaryVertices[j]).data()[2] - point(vertex_handle(i)).data()[2], 2)
				);
				if (minDistance > distance) {
					minDistance = distance;
				}
			}
			double moveDistance = pow(minDistance / maxDistance, swellPower) * swellSize;
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
