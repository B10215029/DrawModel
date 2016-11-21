#include "Triangulation.h"

MyMesh* Triangulation::CreateFace(void* contourPoints, int pointSize, int d)
{
	float* cp = (float*)contourPoints;
	std::vector<CDT::Vertex_handle> ContourPoint;
	CDT m_Triangulation;
	for (int i = 0; i < pointSize; i++) {
		CDT::Vertex_handle vp = m_Triangulation.insert(CDT::Point(cp[i * d + 0], cp[i * d + 1]));
		ContourPoint.push_back(vp);
	}
	for (int i = 0; i < ContourPoint.size(); i++) {
		m_Triangulation.insert_constraint(ContourPoint[i], ContourPoint[(i + 1) % ContourPoint.size()]);
	}
	Mesher mesher(m_Triangulation);
	mesher.refine_mesh();

	mesher.set_criteria(Criteria(0.0125, 50));
	mesher.refine_mesh();

	std::cout << "Number of vertices: " << m_Triangulation.number_of_vertices() << std::endl;
	std::cout << "Number of finite faces: " << m_Triangulation.number_of_faces() << std::endl;

	//glm::vec2 t[3];
	//std::vector<glm::vec2[3]> m_Triangles;
	//m_Triangles.clear();
	//Triangulation::Finite_faces_iterator fc = m_Triangulation.finite_faces_begin();
	//for (; fc != m_Triangulation.finite_faces_end(); ++fc)
	//{
	//	t[0] = glm::vec2((int)fc->vertex(0)->point()[0], (int)fc->vertex(0)->point()[1]);
	//	t[1] = glm::vec2((int)fc->vertex(1)->point()[0], (int)fc->vertex(1)->point()[1]);
	//	t[2] = glm::vec2((int)fc->vertex(2)->point()[0], (int)fc->vertex(2)->point()[1]);
	//	m_Triangles.push_back(t);
	//}

	double a1 = cp[3] - cp[0];
	double b1 = cp[4] - cp[1];
	double c1 = cp[5] - cp[2];
	double a2 = cp[pointSize * d - 3] - cp[0];
	double b2 = cp[pointSize * d - 2] - cp[1];
	double c2 = cp[pointSize * d - 1] - cp[2];
	double zx = (b2 * c1 - b1 * c2) / (b2 * a1 - b1 * a2);
	double zy = (a2 * c1 - a1 * c2) / (a2 * b1 - a1 * b2);
	for (int i = 0; i < pointSize; i++) {
		printf("p%d: %lf, %lf, %lf\n", i, cp[i * 3 + 0], cp[i * 3 + 1], cp[i * 3 + 2]);
	}
	printf("a1: %lf\n", a1);
	printf("b1: %lf\n", b1);
	printf("c1: %lf\n", c1);
	printf("a2: %lf\n", a2);
	printf("b2: %lf\n", b2);
	printf("c2: %lf\n", c2);
	printf("zx:%f, zy:%f\n", zx, zy);
	MyMesh* mesh = new MyMesh();
	MyMesh::VertexHandle vh[3];
	double thick = 0.3;
	std::vector<MyMesh::VertexHandle> vertsOM;
	std::vector<CDT::Vertex_handle> vertsCGAL;
	for (CDT::Finite_vertices_iterator vc = m_Triangulation.finite_vertices_begin(); vc != m_Triangulation.finite_vertices_end(); ++vc) {
		vertsOM.push_back(mesh->add_vertex(MyMesh::Point(vc->point()[0], vc->point()[1], (vc->point()[0] - cp[0]) * zx + (vc->point()[1] - cp[1]) * zy + cp[2])));
		vertsCGAL.push_back(vc->handle());
		//printf("vh: %d\n", vc->handle());
	}

	for (CDT::Finite_faces_iterator fc = m_Triangulation.finite_faces_begin(); fc != m_Triangulation.finite_faces_end(); ++fc) {
		vh[0] = vertsOM[std::find(vertsCGAL.begin(), vertsCGAL.end(), fc->vertex(0)->handle()) - vertsCGAL.begin()];
		vh[1] = vertsOM[std::find(vertsCGAL.begin(), vertsCGAL.end(), fc->vertex(1)->handle()) - vertsCGAL.begin()];
		vh[2] = vertsOM[std::find(vertsCGAL.begin(), vertsCGAL.end(), fc->vertex(2)->handle()) - vertsCGAL.begin()];
		//vh[0] = mesh->add_vertex(MyMesh::Point(fc->vertex(0)->point()[0], fc->vertex(0)->point()[1], (fc->vertex(0)->point()[0] - cp[0]) * zx + (fc->vertex(0)->point()[1] - cp[1]) * zy + cp[2]));
		//vh[1] = mesh->add_vertex(MyMesh::Point(fc->vertex(1)->point()[0], fc->vertex(1)->point()[1], (fc->vertex(1)->point()[0] - cp[0]) * zx + (fc->vertex(1)->point()[1] - cp[1]) * zy + cp[2]));
		//vh[2] = mesh->add_vertex(MyMesh::Point(fc->vertex(2)->point()[0], fc->vertex(2)->point()[1], (fc->vertex(2)->point()[0] - cp[0]) * zx + (fc->vertex(2)->point()[1] - cp[1]) * zy + cp[2]));
		mesh->add_face(vh, 3);
		//vh[0] = mesh->add_vertex(MyMesh::Point(fc->vertex(0)->point()[0], fc->vertex(0)->point()[1], (fc->vertex(0)->point()[0] - cp[0]) * zx + (fc->vertex(0)->point()[1] - cp[1]) * zy + cp[2] - thick));
		//vh[1] = mesh->add_vertex(MyMesh::Point(fc->vertex(2)->point()[0], fc->vertex(2)->point()[1], (fc->vertex(2)->point()[0] - cp[0]) * zx + (fc->vertex(2)->point()[1] - cp[1]) * zy + cp[2] - thick));
		//vh[2] = mesh->add_vertex(MyMesh::Point(fc->vertex(1)->point()[0], fc->vertex(1)->point()[1], (fc->vertex(1)->point()[0] - cp[0]) * zx + (fc->vertex(1)->point()[1] - cp[1]) * zy + cp[2] - thick));
		//mesh->add_face(vh, 3);
	}
	mesh->update_normals();
	
	if (!OpenMesh::IO::write_mesh(*mesh, "test.obj")) {
		std::cerr << "write error\n";
	}
	printf("CreateFace OK\n");
	printf("v:%d, f:%d", mesh->n_vertices(), mesh->n_faces());
	return mesh;
}

int PointIntersectContour()
{
	return 0;
}