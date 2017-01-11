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
			int id;
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
	controlPoint.clear();
	facePointSet.clear();
	for (MyMesh::FaceIter f_it = mesh.faces_begin(); f_it != mesh.faces_end(); ++f_it) {
		for (MyMesh::FaceVertexIter fv_it = mesh.fv_begin(f_it); fv_it != mesh.fv_end(f_it); ++fv_it) {
			MyMesh::Point p = mesh.point(fv_it);
			facePointSet.push_back(glm::vec3(p[0], p[1], p[2]));
		}
	}
}

void ARAPPanel::SelectPoint(int id)
{
	if (std::find(controlPoint.begin(), controlPoint.end(), id) == controlPoint.end()) {
		std::cout << "Select point " << id << std::endl;
		if (animating)
			animationOutputStream << "s " << id << std::endl;
		controlPoint.push_back(id);
	}
}

void ARAPPanel::UnselectPoint(int id)
{
	if (std::find(controlPoint.begin(), controlPoint.end(), id) != controlPoint.end()) {
		std::cout << "Unselect point " << id << std::endl;
		if (animating)
			animationOutputStream << "u " << id << std::endl;
		controlPoint.erase(std::find(controlPoint.begin(), controlPoint.end(), id));
	}
}

void ARAPPanel::MovePoint(int id, glm::vec2 newPos)
{
	std::cout << "Move point " << id << " to " << newPos.x << ", " << newPos.y << std::endl;
	if (animating)
		animationOutputStream << "m " << id << " " << newPos.x << " " << newPos.y << std::endl;
	mesh.point(MyMesh::VertexHandle(id))[0] = newPos.x;
	mesh.point(MyMesh::VertexHandle(id))[1] = newPos.y;
	facePointSet.clear();
	for (MyMesh::FaceIter f_it = mesh.faces_begin(); f_it != mesh.faces_end(); ++f_it) {
		for (MyMesh::FaceVertexIter fv_it = mesh.fv_begin(f_it); fv_it != mesh.fv_end(f_it); ++fv_it) {
			MyMesh::Point p = mesh.point(fv_it);
			facePointSet.push_back(glm::vec3(p[0], p[1], p[2]));
		}
	}
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
