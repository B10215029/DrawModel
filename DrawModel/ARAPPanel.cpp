#pragma unmanaged
#include "ARAPPanel.h"
#include "ShaderUtility.h"
#include "resource.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

ARAPPanel::ARAPPanel()
{
	part = NULL;
	zoom = 1;
	selectPoint = -1;
	animating = false;
}

ARAPPanel::~ARAPPanel()
{
}

void ARAPPanel::Initialize()
{
	//glEnable(GL_CULL_FACE);
	//glEnable(GL_DEPTH_TEST);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
	drawColor.program = loadProgram(IDR_SHADER6, IDR_SHADER5);
	drawColor.modelMatrixLocation = glGetUniformLocation(drawColor.program, "model_matrix");
	drawColor.viewMatrixLocation = glGetUniformLocation(drawColor.program, "view_matrix");
	drawColor.projectionMatrixLocation = glGetUniformLocation(drawColor.program, "projection_matrix");
	drawColor.colorLocation = glGetUniformLocation(drawColor.program, "color");
}

void ARAPPanel::Reshape(int width, int height)
{
	glViewport(0, 0, width, height);
}

void ARAPPanel::Display()
{
	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!part)
		return;
	if (facePointSet.size() == 0)
		SetPart(part);
	if (facePointSet.size() == 0)
		return;
	glm::mat4 modelMat;
	modelMat = glm::scale(glm::mat4(1), glm::vec3(1.0 / zoom, 1.0 / zoom, 1));

	glUseProgram(drawColor.program);
	glUniformMatrix4fv(drawColor.modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMat));
	glUniformMatrix4fv(drawColor.viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));
	glUniformMatrix4fv(drawColor.projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));
	glEnableVertexAttribArray(0);

	glLineWidth(1);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glUniform4fv(drawColor.colorLocation, 1, glm::value_ptr(glm::vec4(0, 0, 0, 1)));
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, &facePointSet[0]);
	glDrawArrays(GL_TRIANGLES, 0, facePointSet.size());

	glPointSize(5);
	glUniform4fv(drawColor.colorLocation, 1, glm::value_ptr(glm::vec4(0, 0, 1, 1)));
	for (int i = 0; i < controlPoint.size(); i++) {
		glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 0, mesh.point(MyMesh::VertexHandle(controlPoint[i])).data());
		glDrawArrays(GL_POINTS, 0, 1);
	}
	char c;
	if (animationInputStream.good() && animationInputStream.get(c)) {
		if (c == 's') {
			int id;
			animationInputStream >> id;
			std::cout << c << id << std::endl;
			SelectPoint(id);
		}
		if (c == 'u') {
			int id;
			animationInputStream >> id;
			std::cout << c << id << std::endl;
			UnselectPoint(id);
		}
		if (c == 'm') {
			int id;
			float x, y;
			animationInputStream >> id >> x >> y;
			std::cout << c << id << "(" << x << ", " << y << ")" << std::endl;
			MovePoint(id, glm::vec2(x, y));
		}
		if (c == 'a') {
			std::cout << c << std::endl;
			ApplyToPart();
		}
	}
	else {
		animationInputStream.close();
	}
}

void ARAPPanel::MouseDown(int x, int y, int button)
{
	if (button == 0) {
		glm::vec2 screenPos((float)x / width * 2 - 1, (float)(height - y) / height * 2 - 1);
		int nearestPoint;
		float nearestDistance = FLT_MAX;
		std::vector<glm::vec3> &points = part->getContourScreenPoint();
		for (MyMesh::VertexIter vh = mesh.vertices_begin(); vh != mesh.vertices_end(); ++vh) {
			float dist = glm::length(screenPos - glm::vec2(mesh.point(vh)[0], mesh.point(vh)[1]));
			if (dist < nearestDistance) {
				nearestPoint = vh->idx();
				nearestDistance = dist;
			}
		}
		if (nearestDistance < 0.05) {
			selectPoint = nearestPoint;
			SelectPoint(selectPoint);
		}
		else {
			selectPoint = -1;
		}
	}
	if (button == 1) {
		glm::vec2 screenPos((float)x / width * 2 - 1, (float)(height - y) / height * 2 - 1);
		int nearestPoint;
		float nearestDistance = FLT_MAX;
		std::vector<glm::vec3> &points = part->getContourScreenPoint();
		for (MyMesh::VertexIter vh = mesh.vertices_begin(); vh != mesh.vertices_end(); ++vh) {
			float dist = glm::length(screenPos - glm::vec2(mesh.point(vh)[0], mesh.point(vh)[1]));
			if (dist < nearestDistance) {
				nearestPoint = vh->idx();
				nearestDistance = dist;
			}
		}
		if (nearestDistance < 0.05) {
			selectPoint = -1;
			UnselectPoint(nearestPoint);
		}
	}

}

void ARAPPanel::MouseUp(int x, int y, int button)
{
	if (selectPoint != -1) {
		//part->UpdateContourPoint(selectPoint);
		selectPoint = -1;
	}
	if (button == 2) {
		ApplyToPart();
	}
}

void ARAPPanel::MouseMove(int x, int y)
{
	if (selectPoint != -1) {
		glm::vec2 screenPos((float)x / width * 2 - 1, (float)(height - y) / height * 2 - 1);
		//std::vector<glm::vec3> &points = part->getContourScreenPoint();
		MovePoint(selectPoint, screenPos);
	}
}

void ARAPPanel::MouseWheel(int x, int y, int delta)
{
	if (delta < 0) {
		zoom++;
	}
	else {
		zoom--;
		if (zoom < 1)
			zoom = 1;
	}
}

void ARAPPanel::SetPart(ModelPart* modelPart)
{
	part = modelPart;
	if (!part)
		return;
	mesh = part->GetPlane();
	baseMesh = mesh;
	controlPoint.clear();
	flags.clear();
	flags.resize(mesh.n_vertices(), false);
	facePointSet.clear();
	for (MyMesh::FaceIter f_it = mesh.faces_begin(); f_it != mesh.faces_end(); ++f_it) {
		for (MyMesh::FaceVertexIter fv_it = mesh.fv_begin(f_it); fv_it != mesh.fv_end(f_it); ++fv_it) {
			MyMesh::Point p = mesh.point(fv_it);
			facePointSet.push_back(glm::vec3(p[0], p[1], p[2]));
		}
	}
	PreComputeG();
	PreComputeF();
	PreComputeH();
}

void ARAPPanel::SelectPoint(int id)
{
	if (!flags[id]) {
		std::cout << "Select point " << id << std::endl;
		if (animating)
			animationOutputStream << "s " << id << std::endl;
		controlPoint.push_back(id);
		flags[id] = true;
		PreStep1();
		PreStep2();
	}
}

void ARAPPanel::UnselectPoint(int id)
{
	if (flags[id]) {
		std::cout << "Unselect point " << id << std::endl;
		if (animating)
			animationOutputStream << "u " << id << std::endl;
		controlPoint.erase(std::find(controlPoint.begin(), controlPoint.end(), id));
		flags[id] = false;
		PreStep1();
		PreStep2();
	}
}

void ARAPPanel::MovePoint(int id, glm::vec2 newPos)
{
	std::cout << "Move point " << id << " to " << newPos.x << ", " << newPos.y << std::endl;
	if (animating)
		animationOutputStream << "m " << id << " " << newPos.x << " " << newPos.y << std::endl;
	mesh.point(MyMesh::VertexHandle(id))[0] = newPos.x;
	mesh.point(MyMesh::VertexHandle(id))[1] = newPos.y;
	Deformation();
	facePointSet.clear();
	for (MyMesh::FaceIter f_it = mesh.faces_begin(); f_it != mesh.faces_end(); ++f_it) {
		for (MyMesh::FaceVertexIter fv_it = mesh.fv_begin(f_it); fv_it != mesh.fv_end(f_it); ++fv_it) {
			MyMesh::Point p = mesh.point(fv_it);
			facePointSet.push_back(glm::vec3(p[0], p[1], p[2]));
		}
	}
	//fast apply
}

void ARAPPanel::ApplyToPart()
{
	std::cout << "Apply to part" << std::endl;
	if (animating)
		animationOutputStream << "a" << std::endl;
	part->SetPlane(mesh);
	part->ReExtrude();
	part->invalidateBuffer = true;
}

void ARAPPanel::ReadAnimation(const char* fileName)
{
	animationInputStream.open(fileName);
}

void ARAPPanel::StartOutputAnimation(const char* fileName)
{
	animationOutputStream.open(fileName);
	animating = true;
}

void ARAPPanel::EndOutputAnimation()
{
	animationOutputStream.close();
	animating = false;
}

void ARAPPanel::GetGeachTri(MyMesh::FaceHandle fh, Eigen::Matrix<double, 6, 6> &G)
{
	// G = A'A
	Eigen::Matrix<double, 2, 6> A;
	Eigen::Vector2d v[3];
	G = Eigen::Matrix<double, 6, 6>::Zero();

	int i = 0;
	for (MyMesh::FaceVertexIter fv_it = mesh.fv_begin(fh); fv_it != mesh.fv_end(fh); fv_it++, i++) {
		v[i] = Eigen::Vector2d(mesh.point(fv_it)[0], mesh.point(fv_it)[1]);
	}

	for (int i = 0; i < 3; i++) {
		A = Eigen::Matrix<double, 2, 6>::Zero();

		int i0 = i, i1 = (i + 1) % 3, i2 = (i + 2) % 3;


		// First compute x01 and y01;
		//	xdir = v1-v0, ydir = (-xdir[1], xdir[0]), v2dir = v2-v0
		//  xdir[0] * x01 + ydir[0] * y01 = v2dir[0]
		//  xdir[1] * x01 + ydir[1] * y01 = v2dir[1]
		//

		Eigen::Vector2d xdir = v[i1] - v[i0];
		Eigen::Vector2d ydir(-xdir[1], xdir[0]);
		Eigen::Vector2d v2dir = v[i2] - v[i0];
		double det = xdir[0] * ydir[1] - xdir[1] * ydir[0];
		double x01 = (v2dir[0] * ydir[1] - v2dir[1] * ydir[0]) / det;
		double y01 = (xdir[0] * v2dir[1] - xdir[1] * v2dir[0]) / det;

		// Now ready for A

		// for v0'
		A(0, i0 * 2 + 0) = 1 - x01;
		A(1, i0 * 2 + 0) = -y01;
		A(0, i0 * 2 + 1) = y01;
		A(1, i0 * 2 + 1) = 1 - x01;

		// for v1'
		A(0, i1 * 2 + 0) = x01;
		A(1, i1 * 2 + 0) = y01;
		A(0, i1 * 2 + 1) = -y01;
		A(1, i1 * 2 + 1) = x01;

		// for v2'
		A(0, i2 * 2 + 0) = -1;
		A(1, i2 * 2 + 0) = 0;
		A(0, i2 * 2 + 1) = 0;
		A(1, i2 * 2 + 1) = -1;

		G = G + A.transpose() * A;
	}
}

void ARAPPanel::GetFeachTri(MyMesh::FaceHandle fh, Eigen::Matrix<double, 6, 4> &K, Eigen::Matrix<double, 4, 4> &F)
{
	// F = K'K

	// Reset F
	K.setZero();
	K(0, 0) = K(1, 1) = K(2, 2) = K(3, 3) = 1;

	Eigen::Vector2d v[3];
	int i = 0;
	for (MyMesh::FaceVertexIter fv_it = mesh.fv_begin(fh); fv_it != mesh.fv_end(fh); fv_it++, i++) {
		v[i] = Eigen::Vector2d(mesh.point(fv_it)[0], mesh.point(fv_it)[1]);
	}

	// First compute x01 and y01;
	//	xdir = v1-v0, ydir = (-xdir[1], xdir[0]), v2dir = v2-v0
	//  xdir[0] * x01 + ydir[0] * y01 = v2dir[0]
	//  xdir[1] * x01 + ydir[1] * y01 = v2dir[1]
	//

	Eigen::Vector2d xdir = v[1] - v[0];
	Eigen::Vector2d ydir(-xdir[1], xdir[0]);
	Eigen::Vector2d v2dir = v[2] - v[0];
	double det = xdir[0] * ydir[1] - xdir[1] * ydir[0];
	double x01 = (v2dir[0] * ydir[1] - v2dir[1] * ydir[0]) / det;
	double y01 = (xdir[0] * v2dir[1] - xdir[1] * v2dir[0]) / det;

	// Now ready for F

	// for v0'
	K(4, 0) = 1 - x01;
	K(5, 0) = -y01;
	K(4, 1) = y01;
	K(5, 1) = 1 - x01;

	// for v1'
	K(4, 2) = x01;
	K(5, 2) = y01;
	K(4, 3) = -y01;
	K(5, 3) = x01;

	F = K.transpose() * K;
}

void ARAPPanel::GetHeachTri(MyMesh::FaceHandle fh, Eigen::Matrix<double, 6, 6> &H)
{
	// Reset H
	H.setZero();

	// G = tI'tI
	Eigen::Matrix<double, 2, 6> tI;// [2][6], tIt[6][2], tmpH[6][6];

	for (int i = 0; i < 3; i++) {
		// Reset tI
		tI.setZero();

		int i0 = i, i1 = (i + 1) % 3, i2 = (i + 2) % 3;

		// for v0'
		tI(0, i0 * 2 + 0) = -1;
		tI(1, i0 * 2 + 1) = -1;

		// for v1'
		tI(0, i1 * 2 + 0) = 1;
		tI(1, i1 * 2 + 1) = 1;

		H += tI.transpose() * tI;
	}
}

void ARAPPanel::PreComputeG()
{
	BigG.resize(mesh.n_vertices() * 2, mesh.n_vertices() * 2);
	//for (MyMesh::EdgeIter e_it = mesh.edges_begin(); e_it != mesh.edges_end(); e_it++) {
	//	MyMesh::HalfedgeHandle he = mesh.halfedge_handle(e_it, 0);
	//	int vid0 = mesh.from_vertex_handle(he).idx();
	//	int vid1 = mesh.to_vertex_handle(he).idx();
	//	BigG.insert(vid0 * 2 + 0, vid1 * 2 + 0) = 0;
	//	BigG.insert(vid0 * 2 + 1, vid1 * 2 + 0) = 0;
	//	BigG.insert(vid0 * 2 + 0, vid1 * 2 + 1) = 0;
	//	BigG.insert(vid0 * 2 + 1, vid1 * 2 + 1) = 0;
	//	BigG.insert(vid1 * 2 + 0, vid0 * 2 + 0) = 0;
	//	BigG.insert(vid1 * 2 + 1, vid0 * 2 + 0) = 0;
	//	BigG.insert(vid1 * 2 + 0, vid0 * 2 + 1) = 0;
	//	BigG.insert(vid1 * 2 + 1, vid0 * 2 + 1) = 0;
	//}
	//for (MyMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); v_it++) {
	//	int vid = v_it->idx();
	//	BigG.insert(vid * 2 + 0, vid * 2 + 0) = 0;
	//	BigG.insert(vid * 2 + 1, vid * 2 + 0) = 0;
	//	BigG.insert(vid * 2 + 0, vid * 2 + 1) = 0;
	//	BigG.insert(vid * 2 + 1, vid * 2 + 1) = 0;
	//}
	for (MyMesh::FaceIter f_it = mesh.faces_begin(); f_it != mesh.faces_end(); f_it++) {
		Eigen::Matrix<double, 6, 6> G;
		int vid[3], vidi = 0;
		GetGeachTri(f_it, G);
		for (MyMesh::FaceVertexIter fv_it = mesh.fv_begin(f_it); fv_it != mesh.fv_end(f_it); fv_it++) {
			vid[vidi++] = fv_it->idx();
		}
		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 3; k++) {
				BigG.coeffRef(vid[j] * 2 + 0, vid[k] * 2 + 0) += G(j * 2 + 0, k * 2 + 0);
				BigG.coeffRef(vid[j] * 2 + 1, vid[k] * 2 + 0) += G(j * 2 + 1, k * 2 + 0);
				BigG.coeffRef(vid[j] * 2 + 0, vid[k] * 2 + 1) += G(j * 2 + 0, k * 2 + 1);
				BigG.coeffRef(vid[j] * 2 + 1, vid[k] * 2 + 1) += G(j * 2 + 1, k * 2 + 1);
			}
		}
	}
}

void ARAPPanel::PreComputeF()
{
	// For each triangle
	int nt = mesh.n_faces();;
	F.resize(nt);
	invF.resize(nt);
	K.resize(nt);

	int i = 0;
	for (MyMesh::FaceIter f_it = mesh.faces_begin(); f_it != mesh.faces_end(); f_it++, i++) {
		GetFeachTri(f_it, K[i], F[i]);
		invF[i] = F[i].inverse();
	}
}

void ARAPPanel::PreComputeH()
{
	int nv = flags.size();
	BigH.resize(nv * 2, nv * 2);

	// For each triangle
	for (MyMesh::FaceIter f_it = mesh.faces_begin(); f_it != mesh.faces_end(); f_it++) {
		Eigen::Matrix<double, 6, 6> H;
		GetHeachTri(f_it, H);
		int vid[3], vidi = 0;
		for (MyMesh::FaceVertexIter fv_it = mesh.fv_begin(f_it); fv_it != mesh.fv_end(f_it); fv_it++) {
			vid[vidi++] = fv_it->idx();
		}

		for (int j = 0; j < 3; j++)
			for (int k = 0; k < 3; k++) {
				if (H(j * 2 + 0, k * 2 + 0) != 0)
					BigH.coeffRef(vid[j] * 2 + 0, vid[k] * 2 + 0) += H(j * 2 + 0, k * 2 + 0);
				if (H(j * 2 + 1, k * 2 + 0) != 0)
					BigH.coeffRef(vid[j] * 2 + 1, vid[k] * 2 + 0) += H(j * 2 + 1, k * 2 + 0);
				if (H(j * 2 + 0, k * 2 + 1) != 0)
					BigH.coeffRef(vid[j] * 2 + 0, vid[k] * 2 + 1) += H(j * 2 + 0, k * 2 + 1);
				if (H(j * 2 + 1, k * 2 + 1) != 0)
					BigH.coeffRef(vid[j] * 2 + 1, vid[k] * 2 + 1) += H(j * 2 + 1, k * 2 + 1);
			}
	}
}

void ARAPPanel::PreStep1()
{
	// The map from vertex index to index in the matrix (then multiply by 2)
	int nv = mesh.n_vertices(), cur_free = 0, cur_ctrl = 0;
	std::vector<int> vert_map(nv, 0);
	for (int i = 0; i < nv; i++) {
		if (!flags[i])
			vert_map[i] = cur_free++;
		else
			vert_map[i] = cur_ctrl++;
	}

	if (cur_ctrl == 0 || cur_free == 0)
		return;

	G00.resize(cur_free * 2, cur_free * 2);
	G01.resize(cur_free * 2, cur_ctrl * 2);
	G10.resize(cur_ctrl * 2, cur_free * 2);
	G11.resize(cur_ctrl * 2, cur_ctrl * 2);

	// For each triangle
	int nt = mesh.n_faces();
	for (MyMesh::FaceIter f_it = mesh.faces_begin(); f_it != mesh.faces_end(); f_it++) {
		int vid[3], vidi = 0;
		for (MyMesh::FaceVertexIter fv_it = mesh.fv_begin(f_it); fv_it != mesh.fv_end(f_it); fv_it++) {
			vid[vidi++] = fv_it->idx();
		}

		// assign G's to G00, G01, G10, G11 accordingly
		for (int j = 0; j < 3; j++)
			for (int k = 0; k < 3; k++) {
				Eigen::SparseMatrix<double>* GG = NULL;
				// Non-zeros could only be the diagonal entries 
				if (!flags[vid[j]] && !flags[vid[k]])
					GG = &G00;
				else if (!flags[vid[j]] && flags[vid[k]])
					GG = &G01;
				else if (flags[vid[j]] && !flags[vid[k]])
					GG = &G10;
				else // if (flags[t[j]] && flags[t[k]])
					GG = &G11;

				GG->coeffRef(vert_map[vid[j]] * 2 + 0, vert_map[vid[k]] * 2 + 0) = BigG.coeffRef(vid[j] * 2 + 0, vid[k] * 2 + 0);
				GG->coeffRef(vert_map[vid[j]] * 2 + 1, vert_map[vid[k]] * 2 + 0) = BigG.coeffRef(vid[j] * 2 + 1, vid[k] * 2 + 0);
				GG->coeffRef(vert_map[vid[j]] * 2 + 0, vert_map[vid[k]] * 2 + 1) = BigG.coeffRef(vid[j] * 2 + 0, vid[k] * 2 + 1);
				GG->coeffRef(vert_map[vid[j]] * 2 + 1, vert_map[vid[k]] * 2 + 1) = BigG.coeffRef(vid[j] * 2 + 1, vid[k] * 2 + 1);
			}
	}
	Gprime = G00.transpose();
	Gprime += G00;
	B = G10.transpose();
	B += G01;
	
	Gprime.makeCompressed();
	linearSolverG.compute(Gprime.transpose() * Gprime);
}

void ARAPPanel::PreStep2()
{
	// Preparing for the memory
	int nt = mesh.n_faces();
	C.resize(nt);
	for (int i = 0; i < nt; i++) {
		C[i].resize(4);
	}
	fittedVertices.resize(nt);
	for (int i = 0; i < nt; i++) {
		fittedVertices[i].resize(3);
	}

	// Compute Hprime and D

	// The map from vertex index to index in the matrix (then multiply by 2)
	int nv = flags.size(), cur_free = 0, cur_ctrl = 0;
	std::vector<int> vert_map(nv, 0);
	for (int i = 0; i < nv; i++) {
		if (!flags[i])
			vert_map[i] = cur_free++;
		else
			vert_map[i] = cur_ctrl++;
	}

	if (cur_ctrl == 0 || cur_free == 0)
		return;

	H00.resize(cur_free * 2, cur_free * 2);
	H01.resize(cur_free * 2, cur_ctrl * 2);
	H10.resize(cur_ctrl * 2, cur_free * 2);
	H11.resize(cur_ctrl * 2, cur_ctrl * 2);

	// For each triangle
	for (MyMesh::FaceIter f_it = mesh.faces_begin(); f_it != mesh.faces_end(); f_it++) {
		int vid[3], vidi = 0;
		for (MyMesh::FaceVertexIter fv_it = mesh.fv_begin(f_it); fv_it != mesh.fv_end(f_it); fv_it++) {
			vid[vidi++] = fv_it->idx();
		}

		// assign H's to H00, H01, H10, H11 accordingly
		for (int j = 0; j < 3; j++)
			for (int k = 0; k < 3; k++) {
				Eigen::SparseMatrix<double>* HH = NULL;
				// Non-zeros could only be the diagonal entries 
				if (!flags[vid[j]] && !flags[vid[k]])
					HH = &H00;
				else if (!flags[vid[j]] && flags[vid[k]])
					HH = &H01;
				else if (flags[vid[j]] && !flags[vid[k]])
					HH = &H10;
				else // if (flags[t[j]] && flags[t[k]])
					HH = &H11;

				HH->coeffRef(vert_map[vid[j]] * 2 + 0, vert_map[vid[k]] * 2 + 0) = BigH.coeffRef(vid[j] * 2 + 0, vid[k] * 2 + 0);
				HH->coeffRef(vert_map[vid[j]] * 2 + 1, vert_map[vid[k]] * 2 + 0) = BigH.coeffRef(vid[j] * 2 + 1, vid[k] * 2 + 0);
				HH->coeffRef(vert_map[vid[j]] * 2 + 0, vert_map[vid[k]] * 2 + 1) = BigH.coeffRef(vid[j] * 2 + 0, vid[k] * 2 + 1);
				HH->coeffRef(vert_map[vid[j]] * 2 + 1, vert_map[vid[k]] * 2 + 1) = BigH.coeffRef(vid[j] * 2 + 1, vid[k] * 2 + 1);
			}
	}

	Hprime = H00.transpose();
	Hprime += H00;
	D = H10.transpose();
	D += H01;

	Hprime.makeCompressed();
	linearSolverH.compute(Hprime.transpose() * Hprime);
}

void ARAPPanel::Step1()
{
	Eigen::MatrixXd q(controlPoint.size() * 2, 1), Bq;
	
	int i = 0;
	for (MyMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); v_it++) {
		if (!flags[v_it->idx()])
			continue;
		q(i++) = mesh.point(v_it)[0];
		q(i++) = mesh.point(v_it)[1];
	}

	Bq = -B*q;

	Eigen::MatrixXd u = linearSolverG.solve(Gprime.transpose() * Bq);
	i = 0;
	for (MyMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); v_it++) {
		if (flags[v_it->idx()])
			continue;
		mesh.point(v_it)[0] = u(i++);
		mesh.point(v_it)[1] = u(i++);
	}
}

void ARAPPanel::Step2()
{
	// Compute C

	// For each triangle
	int nt = mesh.n_faces();

	int i = 0;
	for (MyMesh::FaceIter f_it = mesh.faces_begin(); f_it != mesh.faces_end(); f_it++, i++) {
		Eigen::Matrix<double, 6, 4> _K;
		Eigen::Matrix<double, 4, 6> _Kt;
		_K = K[i];

		_Kt = _K.transpose();

		Eigen::Matrix<double, 6, 1> vprime;
		int vid[3], vidi = 0;
		for (MyMesh::FaceVertexIter fv_it = mesh.fv_begin(f_it); fv_it != mesh.fv_end(f_it); fv_it++, vidi++) {
			vid[vidi] = fv_it->idx();
			vprime(vidi * 2 + 0, 0) = mesh.point(fv_it)[0];
			vprime(vidi * 2 + 1, 0) = mesh.point(fv_it)[1];
		}

		Eigen::Matrix<double, 4, 1> _C_tmp;
		_C_tmp = _Kt * vprime;
		for (int j = 0; j < 4; j++)
			C[i][j] = _C_tmp(j, 0);

		// Compute fitted triangle

		// Also compute the other vertex as well
		Eigen::Matrix<double, 6, 1> vfit;
		vfit = _K * (invF[i] * _C_tmp);

		fittedVertices[i][0](0) = vfit(0, 0);
		fittedVertices[i][0](1) = vfit(1, 0);
		fittedVertices[i][1](0) = vfit(2, 0);
		fittedVertices[i][1](1) = vfit(3, 0);
		fittedVertices[i][2](0) = vfit(4, 0);
		fittedVertices[i][2](1) = vfit(5, 0);

		// Compute scale and scale back to get congruent triangles
		// The paper doesn't say anything about how to scale, so I assue
		// it's around the gravity center
		Eigen::Vector2d center = (fittedVertices[i][0] + fittedVertices[i][1] + fittedVertices[i][2]) / 3;
		double scale = (fittedVertices[i][0] - fittedVertices[i][1]).norm() +
			           (fittedVertices[i][1] - fittedVertices[i][2]).norm() +
			           (fittedVertices[i][2] - fittedVertices[i][0]).norm();
		scale /= (baseMesh.point(MyMesh::VertexHandle(vid[0])) - baseMesh.point(MyMesh::VertexHandle(vid[1]))).norm() +
			     (baseMesh.point(MyMesh::VertexHandle(vid[1])) - baseMesh.point(MyMesh::VertexHandle(vid[2]))).norm() +
			     (baseMesh.point(MyMesh::VertexHandle(vid[2])) - baseMesh.point(MyMesh::VertexHandle(vid[0]))).norm();
		

		for (int j = 0; j < 3; j++) {
			Eigen::Vector2d v = (fittedVertices[i][j] - center) / scale;
			fittedVertices[i][j] = center + v;
		}
	}

	//if (step1_only) return;
	// The second sub-step of step2

	// Compute q
	Eigen::MatrixXd q(controlPoint.size() * 2, 1), Dq_plus_f0;
	int nv = mesh.n_vertices(), cur_free = 0, cur_ctrl = 0;

	std::vector<int> vert_map(nv, 0);
	for (int i = 0, qi = 0; i < nv; i++) {
		if (!flags[i])
			vert_map[i] = cur_free++;
		else {
			vert_map[i] = cur_ctrl++;
			q(qi++) = mesh.point(MyMesh::VertexHandle(i))[0];
			q(qi++) = mesh.point(MyMesh::VertexHandle(i))[1];
		}
	}

	if (cur_ctrl == 0 || cur_free == 0)
		return;


	// Compute f0 and f1
	Eigen::MatrixXd f(nv * 2, 1), f0(cur_free * 2, 1), f1(cur_ctrl * 2, 1);
	f.setZero();
	for (MyMesh::FaceIter f_it = mesh.faces_begin(); f_it != mesh.faces_end(); f_it++) {
		int vid[3], vidi = 0;
		for (MyMesh::FaceVertexIter fv_it = mesh.fv_begin(f_it); fv_it != mesh.fv_end(f_it); fv_it++) {
			vid[vidi++] = fv_it->idx();
		}
		std::vector<Eigen::Vector2d>& fitted = fittedVertices[f_it->idx()];
		for (int i = 0; i < 3; i++) {
			int j = (i + 1) % 3;
			Eigen::Vector2d vij_f = fitted[j] - fitted[i];
			f(2 * vid[i] + 0, 0) += -2 * vij_f[0];
			f(2 * vid[i] + 1, 0) += -2 * vij_f[1];
			f(2 * vid[j] + 0, 0) +=  2 * vij_f[0];
			f(2 * vid[j] + 1, 0) +=  2 * vij_f[1];
		}
	}
	// Map them into f0, f1
	for (int i = 0; i < nv; i++) {
		if (flags[i] == 0) {
			f0(2 * vert_map[i] + 0, 0) = f(2 * i + 0, 0);
			f0(2 * vert_map[i] + 1, 0) = f(2 * i + 1, 0);
		}
		else {
			f1(2 * vert_map[i] + 0, 0) = f(2 * i + 0, 0);
			f1(2 * vert_map[i] + 1, 0) = f(2 * i + 1, 0);
		}
	}

	// This is Dq
	Dq_plus_f0 = f0 - D*q;

	Eigen::MatrixXd u = linearSolverH.solve(Hprime.transpose() * Dq_plus_f0);
	i = 0;
	for (MyMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); v_it++) {
		if (flags[v_it->idx()])
			continue;
		//printf("%+02.3lf, %+02.3lf -> %+02.3lf, %+02.3lf : %+02.3lf, %+02.3lf\n", mesh.point(v_it)[0], mesh.point(v_it)[1], u(i), u(i + 1), mesh.point(v_it)[0] - u(i), mesh.point(v_it)[1] - u(i + 1));
		mesh.point(v_it)[0] = u(i++);
		mesh.point(v_it)[1] = u(i++);
	}
}

void ARAPPanel::Deformation()
{
	Step1();
	Step2();
}

