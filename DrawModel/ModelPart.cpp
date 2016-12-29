#include "ModelPart.h"
#include "ShaderUtility.h"
#include "Triangulation.h"
#include "resource.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct ModelPart::DrawTextureProgram ModelPart::drawTexture = {};
struct ModelPart::DrawColorProgram ModelPart::drawColor = {};
struct ModelPart::DrawStrokeProgram ModelPart::drawStroke = {};
struct ModelPart::DrawSolidProgram ModelPart::drawSolid = {};
glm::vec4 ModelPart::strokeColor(0, 0, 1, 1);
float ModelPart::strokeSize(50);
float ModelPart::strokeInterval(0.005f);
float ModelPart::contourInterval(0.05f);
GLuint ModelPart::strokeTextureHandle;
glm::mat4 ModelPart::modelMatrix;
glm::mat4 ModelPart::viewMatrix;
glm::mat4 ModelPart::projectionMatrix;
float ModelPart::extrudeThickness = 1;
float ModelPart::extrudeDivisions = 3;
float ModelPart::extrudeOffset = 0;
float ModelPart::swellPower = 1;
float ModelPart::swellSize = 0;
bool ModelPart::modelRenderPoint = true;
bool ModelPart::modelRenderLine = true;
float ModelPart::triAspect = 0.0125;
float ModelPart::triSize = 50;

void ModelPart::InitProgram()
{
	drawTexture.program = loadProgram(IDR_SHADER3, IDR_SHADER4);
	drawTexture.textureLocation = glGetUniformLocation(drawTexture.program, "image");
	drawTexture.flipYLocation = glGetUniformLocation(drawTexture.program, "flipY");
	drawColor.program = loadProgram(IDR_SHADER6, IDR_SHADER5);
	drawColor.modelMatrixLocation = glGetUniformLocation(drawColor.program, "model_matrix");
	drawColor.viewMatrixLocation = glGetUniformLocation(drawColor.program, "view_matrix");
	drawColor.projectionMatrixLocation = glGetUniformLocation(drawColor.program, "projection_matrix");
	drawColor.colorLocation = glGetUniformLocation(drawColor.program, "color");
	drawStroke.program = loadProgram(IDR_SHADER8, IDR_SHADER7);
	drawStroke.strokeSizeLocation = glGetUniformLocation(drawStroke.program, "strokeSize");
	drawStroke.colorLocation = glGetUniformLocation(drawStroke.program, "color");
	drawStroke.colorTextureLocation = glGetUniformLocation(drawStroke.program, "colorTexture");
	drawStroke.strokeTextureLocation = glGetUniformLocation(drawStroke.program, "strokeTexture");
	drawStroke.modelMatrixLocation = glGetUniformLocation(drawStroke.program, "model_matrix");
	drawStroke.viewMatrixLocation = glGetUniformLocation(drawStroke.program, "view_matrix");
	drawStroke.projectionMatrixLocation = glGetUniformLocation(drawStroke.program, "projection_matrix");
	drawSolid.program = loadProgram(IDR_SHADER1, IDR_SHADER2);
	drawSolid.modelMatrixLocation = glGetUniformLocation(drawSolid.program, "model_matrix");
	drawSolid.viewMatrixLocation = glGetUniformLocation(drawSolid.program, "view_matrix");
	drawSolid.projectionMatrixLocation = glGetUniformLocation(drawSolid.program, "projection_matrix");
	drawSolid.colorLocation = glGetUniformLocation(drawSolid.program, "color");
	drawSolid.textureLocation = glGetUniformLocation(drawSolid.program, "tex0");
	drawSolid.useTextureLocation = glGetUniformLocation(drawSolid.program, "useTexture");
	//drawSolid.textureHandle = loadTextureFromFilePNG("grid32.png");
	//drawSolid.textureHandle = loadTextureFromFilePNG("testtexture.png");
	//strokeTextureHandle = loadTextureFromFilePNG("./stroke/stroke1.png");
	//modelMatrix = glm::mat4(1.0f);
	//viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -1.5));
	//projectionMatrix = glm::perspective(glm::radians(45.0f), (float)800 / (float)800, 0.1f, 10000.f);;
}

void ModelPart::SetStrokeTextureFromFile(const char* filePath)
{
	ModelPart::strokeTextureHandle = loadTextureFromFilePNG(filePath);
}

void ModelPart::SavePartsToOBJ(std::deque<ModelPart*> parts, const char* fileName)
{
	MyMesh mesh;
	OpenMesh::IO::Options opt(OpenMesh::IO::Options::VertexNormal | OpenMesh::IO::Options::VertexTexCoord | OpenMesh::IO::Options::FaceTexCoord);
	std::deque<MyMesh::VertexHandle> verts;
	MyMesh::VertexHandle vh[3];
	for (int i = 0; i < parts.size(); i++) {
		if (!parts[i]->mesh)
			continue;
		verts.clear();
		for (MyMesh::VertexIter v_it = parts[i]->mesh->vertices_begin(); v_it != parts[i]->mesh->vertices_end(); ++v_it) {
			verts.push_back(mesh.add_vertex(parts[i]->mesh->point(v_it)));
			mesh.set_normal(verts.back(), parts[i]->mesh->normal(v_it));
			mesh.set_texcoord2D(verts.back(), parts[i]->mesh->texcoord2D(v_it));
		}
		for (MyMesh::FaceIter f_it = parts[i]->mesh->faces_begin(); f_it != parts[i]->mesh->faces_end(); ++f_it) {
			MyMesh::FaceVertexIter fv_it = parts[i]->mesh->fv_begin(f_it);
			for (int j = 0; fv_it != parts[i]->mesh->fv_end(f_it); j++, ++fv_it) {
				vh[j] = verts[fv_it->idx()];
			}
			mesh.add_face(vh, 3);
		}
	}
	OpenMesh::IO::write_mesh(mesh, fileName, opt);
}

ModelPart::ModelPart()
{
	mesh = NULL;
	planeMesh = NULL;
	mvp = projectionMatrix * viewMatrix * modelMatrix;
	clearStroke = false;
	drawModelTexture = false;
	state = ModelState::STATE_NONE;
	strokeTextureUV = 0;
	strokeFBOTextureExpired = false;
	strokeFBOTextureData = NULL;
	invalidateBuffer = false;
}

ModelPart::~ModelPart()
{
}

void ModelPart::Render()
{
	if (state == ModelState::STATE_MODEL) {
		if (invalidateBuffer) {
			UpdateMeshBuffer();
			invalidateBuffer = false;
		}
		RenderModel();
		//RenderLine();
		if (clearStroke || !strokePointQueue.empty()) {
			RenderStroke();
		}
	}
	else if (state == ModelState::STATE_DRAWING) {
		RenderStroke();
		RenderModel();
	}
	else {
		RenderContour();
		//RenderStroke();
		//RenderLine();
		//RenderPoint();
	}
}

void ModelPart::RenderStroke()
{
	glBindVertexArray(0);
	if (clearStroke) {
		glBindFramebuffer(GL_FRAMEBUFFER, strokeFBO);
		float clrCol[4];
		glGetFloatv(GL_COLOR_CLEAR_VALUE, clrCol);
		glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(clrCol[0], clrCol[1], clrCol[2], clrCol[3]);
		clearStroke = false;
		if (!drawModelTexture) {
			if (strokeFBOTextureData)
				free(strokeFBOTextureData);
			strokeFBOTextureData = writeTextureToArray(strokeFBOColorTexture);
			strokeFBOTextureExpired = true;
		}
	}
	if (drawModelTexture) {
		glBindFramebuffer(GL_FRAMEBUFFER, strokeFBO);
		glUseProgram(drawTexture.program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, modelTexture);
		glUniform1i(drawTexture.textureLocation, 0);
		glUniform1i(drawTexture.flipYLocation, 0);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		drawModelTexture = false;
		if (strokeFBOTextureData)
			free(strokeFBOTextureData);
		strokeFBOTextureData = writeTextureToArray(strokeFBOColorTexture);
		strokeFBOTextureExpired = true;
	}
	if (!strokePointQueue.empty()) {
		glBindFramebuffer(GL_FRAMEBUFFER, strokeFBO);
		glUseProgram(drawStroke.program);
		glUniform4fv(drawStroke.colorLocation, 1, glm::value_ptr(strokeColor));
		glUniform1f(drawStroke.strokeSizeLocation, strokeSize);
		//glUniformMatrix4fv(drawStroke.modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		//glUniformMatrix4fv(drawStroke.viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		//glUniformMatrix4fv(drawStroke.projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, strokeTextureHandle);
		glUniform1i(drawStroke.strokeTextureLocation, 0);
		glEnableVertexAttribArray(0);
		glm::vec3 *pointTemp = new glm::vec3[strokePointQueue.size()];
		int i;
		for (i = 0; !strokePointQueue.empty(); i++) {
			pointTemp[i] = strokePointQueue.front();
			strokePointQueue.pop();
		}
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, pointTemp);
		glDrawArrays(GL_POINTS, 0, i);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glClear(GL_DEPTH_BUFFER_BIT);
	//glUseProgram(drawTexture.program);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, strokeFBOColorTexture);
	//glUniform1i(drawTexture.textureLocation, 0);
	//glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void ModelPart::RenderLine()
{
	if (!mesh) {
		return;
	}
	glLineWidth(1);
	glBindVertexArray(vao);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(drawSolid.program);
	glUniformMatrix4fv(drawSolid.modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(drawSolid.viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(drawSolid.projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniform1i(drawSolid.useTextureLocation, 0);
	glBindVertexArray(vao);
	glUniform4fv(drawSolid.colorLocation, 1, glm::value_ptr(glm::vec4(0.2, 1.0, 0.2, 1.0)));
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-0.5, -0.5);
	glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ModelPart::RenderPoint()
{
	glPointSize(5);
	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT);
	glUseProgram(drawColor.program);
	glUniform4fv(drawColor.colorLocation, 1, glm::value_ptr(glm::vec4(0, 1, 0, 1)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, &points[0]);
	glDrawArrays(GL_POINTS, 0, points.size());
}

void ModelPart::RenderContour()
{
	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glPointSize(5);
	glUseProgram(drawColor.program);
	glUniformMatrix4fv(drawColor.modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(drawColor.viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(drawColor.projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniform4fv(drawColor.colorLocation, 1, glm::value_ptr(glm::vec4(0, 1, 0, 1)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, &points[0]);
	glDrawArrays(GL_POINTS, 0, points.size());

	glLineWidth(3);
	glUseProgram(drawColor.program);
	glUniformMatrix4fv(drawColor.modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(drawColor.viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(drawColor.projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniform4fv(drawColor.colorLocation, 1, glm::value_ptr(strokeColor));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, &points[0]);
	glDrawArrays(GL_LINE_STRIP, 0, points.size());
}

void ModelPart::RenderUV()
{

}

void ModelPart::RenderModel()
{
	if (!mesh) {
		return;
	}
	glBindVertexArray(vao);
	glUseProgram(drawSolid.program);
	glUniformMatrix4fv(drawSolid.modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(drawSolid.viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(drawSolid.projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniform1i(drawSolid.textureLocation, 0);
	glUniform1i(drawSolid.useTextureLocation, 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, strokeFBOColorTexture);
	glBindVertexArray(vao);
	glUniform4fv(drawSolid.colorLocation, 1, glm::value_ptr(glm::vec4(0.5, 0.5, 0.5, 1.0)));
	glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);
	glUniform1i(drawSolid.useTextureLocation, 0);
	glBindVertexArray(0);
}

void ModelPart::CreateFrameBuffer(int width, int height)
{
	glGenFramebuffers(1, &strokeFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, strokeFBO);
	glGenTextures(1, &strokeFBOColorTexture);
	glBindTexture(GL_TEXTURE_2D, strokeFBOColorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, strokeFBOColorTexture, 0);
	float clrCol[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE, clrCol);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(clrCol[0], clrCol[1], clrCol[2], clrCol[3]);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	strokeFBOTextureWidth = width;
	strokeFBOTextureHeight = height;
	if (strokeFBOTextureData)
		free(strokeFBOTextureData);
	strokeFBOTextureData = writeTextureToArray(strokeFBOColorTexture);
	strokeFBOTextureExpired = true;
}

GLuint ModelPart::getStrokeFBOTexture()
{
	if (strokeFBOTextureExpired) {
		glDeleteTextures(1, &strokeTextureUV);
		strokeTextureUV = loadTextureFromArray(strokeFBOTextureData, strokeFBOTextureWidth, strokeFBOTextureHeight, 4);
		strokeFBOTextureExpired = false;
	}
	return strokeTextureUV;
}

void ModelPart::CreateMesh()
{
	if (!isComputableContour(screenPoints)) {
		state = ModelState::STATE_NONE;
		mesh = NULL;
		return;
	}
	printf("CreateMesh\n");
	//mesh = MyMesh::CreateFace(points);
	//mesh = Triangulation::CreateFace(&points[0], points.size(), triAspect, triSize);
	planeMesh = Triangulation::CreateFace(points, mvp, triAspect, triSize);
	mesh = new MyMesh();
	*mesh = *planeMesh;
	OpenMesh::Vec3d avg(0, 0, 0);
	int bpc = 0;
	for (MyMesh::VertexIter v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it) {
		if (mesh->is_boundary(v_it)) {
			avg += mesh->point(v_it);
			bpc++;
		}
	}
	avg /= bpc;
	MyMesh::VertexHandle minvh(0);
	float mindis = FLT_MAX;
	for (MyMesh::VertexIter v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it) {
		if ((mesh->point(v_it) - avg).norm() < mindis) {
			minvh = v_it;
			mindis = (mesh->point(v_it) - avg).norm();
		}
	}
	mesh->Extrude(extrudeThickness, extrudeDivisions, extrudeOffset, swellSize, swellPower);
	state = ModelState::STATE_MODEL;

	//OpenMesh::IO::Options opt(OpenMesh::IO::Options::VertexNormal | OpenMesh::IO::Options::VertexTexCoord | OpenMesh::IO::Options::FaceTexCoord);
	//OpenMesh::IO::read_mesh(*mesh, "ball2.obj", opt);
	//if (opt.check(OpenMesh::IO::Options::VertexNormal)) {
	//	printf("VertexNormal\n");
	//}
	//if (opt.check(OpenMesh::IO::Options::VertexTexCoord)) {
	//	printf("VertexTexCoord\n");
	//}
	//if (mesh->has_vertex_normals()) {
	//	printf("has_vertex_normals\n");
	//}
	//if (mesh->has_vertex_texcoords2D()) {
	//	printf("has_vertex_texcoords2D\n");
	//}
	//mesh->update_normals();

	mesh->ResetUV();
	mesh->ComputeUV(minvh);
	//mesh->LaplacianSmooth();
}

void ModelPart::ReExtrude()
{
	if (!planeMesh) {
		return;
	}
	mesh = new MyMesh();
	*mesh = *planeMesh;
	OpenMesh::Vec3d avg(0, 0, 0);
	int bpc = 0;
	for (MyMesh::VertexIter v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it) {
		if (mesh->is_boundary(v_it)) {
			avg += mesh->point(v_it);
			bpc++;
		}
	}
	avg /= bpc;
	MyMesh::VertexHandle minvh(0);
	float mindis = FLT_MAX;
	for (MyMesh::VertexIter v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it) {
		if ((mesh->point(v_it) - avg).norm() < mindis) {
			minvh = v_it;
			mindis = (mesh->point(v_it) - avg).norm();
		}
	}
	mesh->Extrude(extrudeThickness, extrudeDivisions, extrudeOffset, swellSize, swellPower);
	state = ModelState::STATE_MODEL;

	mesh->ResetUV();
	mesh->ComputeUV(minvh);
}

void ModelPart::ReadMesh(const char* fileName)
{
	state = ModelState::STATE_MODEL;
	mesh = new MyMesh();
	OpenMesh::IO::Options opt(OpenMesh::IO::Options::VertexNormal | OpenMesh::IO::Options::VertexTexCoord | OpenMesh::IO::Options::FaceTexCoord);
	OpenMesh::IO::read_mesh(*mesh, fileName, opt);
	if (!opt.check(OpenMesh::IO::Options::VertexNormal) && mesh->has_vertex_normals()) {
		printf("No VertexNormal, recompute\n");
		mesh->update_normals();
	}
	if (!opt.check(OpenMesh::IO::Options::VertexTexCoord) && mesh->has_vertex_texcoords2D()) {
		printf("No VertexTexCoord, recompute\n");
		mesh->ResetUV();
		mesh->ComputeUV();
	}
}

void ModelPart::ExtractionMesh(float s)
{
	if (mesh) {
		mesh->Extraction(s);
	}
}

void ModelPart::SmoothMesh(int step)
{
	if (mesh) {
		mesh->Smooth(step);
	}
}

void ModelPart::UpdateNormal()
{
	if (mesh) {
		mesh->update_normals();
	}
}

void ModelPart::UpdateUV(int center)
{
	if (mesh) {
		mesh->ResetUV();
		mesh->ComputeUV(MyMesh::VertexHandle(center));
	}
}

void ModelPart::AddPoint(float x, float y, float z)
{
	AddPoint(glm::vec3(x, y, z));
}

void ModelPart::AddPoint(glm::vec3 point)
{
	glm::vec4 screenPoint = mvp * glm::vec4(point, 1);
	screenPoint /= screenPoint.w;
	if (points.empty()) {
		strokePointQueue.push(point);
		points.push_back(point);
		screenPoints.push_back(screenPoint);
	}
	else {
		float screenLength = glm::length(glm::vec3(screenPoint) - screenPoints.back());
		if (screenLength > contourInterval) {
			//glm::vec3 vector = point - points.back();
			//float lineLength = glm::length(vector);
			//// add interpolation point
			//float interval = lineLength / (int)(screenLength / contourInterval);
			//vector = glm::normalize(vector) * interval;
			//for (int i = 1; i < (int)(screenLength / contourInterval); i ++) {
			//	points.push_back(points.back() + vector);
			//}
			// add point
			points.push_back(point);
			screenPoints.push_back(screenPoint);
			//// add interpolation stroke
			//interval = lineLength / (int)(screenLength / strokeInterval);
			//vector = glm::normalize(vector) * -interval;
			//for (int i = 0; i < (int)(screenLength / strokeInterval); i ++) {
			//	strokePointQueue.push(points.back() + vector * float(i));
			//}
		}
	}
}

double TriangleArea(MyMesh::Point vertex0, MyMesh::Point vertex1, MyMesh::Point vertex2) {
	//MyMesh::Point x = vertex1 - vertex0;
	//MyMesh::Point y = vertex2 - vertex0;
	//double r = 
	//	(x[1] * y[2] - x[2] * y[1])*(x[1] * y[2] - x[2] * y[1]) + 
	//	(x[2] * y[0] - x[0] * y[2])*(x[2] * y[0] - x[0] * y[2]) + 
	//	(x[0] * y[1] - x[1] * y[0])*(x[0] * y[1] - x[1] * y[0]);
	double a = (vertex0 - vertex1).length();
	double b = (vertex1 - vertex2).length();
	double c = (vertex2 - vertex0).length();
	double s = (a + b + c) / 2;
	double r = sqrt(s * (s - a) * (s - b) * (s - c));
	return r;
}

void ModelPart::DrawPoint(glm::vec3 point)
{
	if (state != ModelState::STATE_DRAWING)
		return;

	//float pointLength = glm::length(point - drawingPoint);
	//if (pointLength > contourInterval) {
	//	glm::vec3 vector = point - points.back();
	//	float lineLength = glm::length(vector);
	//	// add interpolation stroke
	//	float interval = lineLength / (int)(screenLength / strokeInterval);
	//	vector = glm::normalize(vector) * -interval;
	//	for (int i = 0; i < (int)(screenLength / strokeInterval); i++) {
	//		strokePointQueue.push(points.back() + vector * float(i));
	//	}
	//}

	MyMesh::Point pointP(point.x, point.y, point.z);
	MyMesh::FaceHandle nearestFace;
	double faceDistance = DBL_MAX;
	for (MyMesh::FaceIter f_it = mesh->faces_begin(); f_it != mesh->faces_end(); ++f_it) {
		MyMesh::Point p[3];
		int i = 0, j = 2;
		for (MyMesh::FaceVertexIter fv_it = mesh->fv_begin(f_it); fv_it != mesh->fv_end(f_it); ++fv_it) {
			p[i++] = mesh->point(fv_it);
		}
		double triArea = TriangleArea(p[0], p[1], p[2]);
		for (i = 0, j = 2; i < 3; j = i++) {
			triArea -= TriangleArea(p[i], p[j], pointP);
		}
		if (abs(triArea) < faceDistance) {
			nearestFace = f_it.handle();
			faceDistance = abs(triArea);
		}
	}
	MyMesh::Point p[3];
	MyMesh::TexCoord2D t[3];
	double w[4] = {};
	int i = 0, j = 2;
	for (MyMesh::FaceHalfedgeIter fh_it = mesh->fh_begin(nearestFace); fh_it != mesh->fh_end(nearestFace); ++fh_it) {
		t[i] = mesh->texcoord2D(fh_it);
		p[i++] = mesh->point(mesh->to_vertex_handle(fh_it));
	}
	//for (i = 0, j = 2; i < 3; j = i++) {
	//	int k = (i + 1) % 3;
	//	double tria = TriangleArea(p[i], p[j], pointP);
	//	w[k] = tria * 2 / (p[i] - p[j]).norm();
	//	//w[k] = (p[k] - pointP).norm();
	//	w[3] += w[k];
	//}
	//MyMesh::TexCoord2D strokePosition = t[0] * (w[0] / w[3]) + t[1] * (w[1] / w[3]) + t[2] * (w[2] / w[3]);
	p[2] -= p[0];
	p[1] -= p[0];
	pointP -= p[0];
	glm::vec3 b1(p[2][0], p[2][1], p[2][2]);
	glm::vec3 b2(p[1][0], p[1][1], p[1][2]);
	glm::vec3 pp(pointP[0], pointP[1], pointP[2]);
	glm::vec2 x = glm::inverse(glm::mat2(glm::dot(b1, b1), glm::dot(b1, b2), glm::dot(b2, b1), glm::dot(b2, b2))) * glm::vec2(glm::dot(b1, pp), glm::dot(b2, pp));
	MyMesh::TexCoord2D strokePosition = t[0] + (t[2] - t[0]) * x[0] + (t[1] - t[0]) * x[1];
	strokePointQueue.push(glm::vec3(strokePosition[0], strokePosition[1], 0));

	drawingPoint = point;
}

void ModelPart::StartDraw(glm::vec3 point)
{
	//double aa = TriangleArea()
	if (state != ModelState::STATE_MODEL)
		return;
	drawingPoint = point;
	state = ModelState::STATE_DRAWING;
	MyMesh::Point pointP(point.x, point.y, point.z);
	MyMesh::FaceHandle nearestFace;
	double faceDistance = DBL_MAX;
	for (MyMesh::FaceIter f_it = mesh->faces_begin(); f_it != mesh->faces_end(); ++f_it) {
		MyMesh::Point p[3];
		int i = 0, j = 2;
		for (MyMesh::FaceVertexIter fv_it = mesh->fv_begin(f_it); fv_it != mesh->fv_end(f_it); ++fv_it) {
			p[i++] = mesh->point(fv_it);
		}
		double triArea = TriangleArea(p[0], p[1], p[2]);
		for (i = 0, j = 2; i < 3; j = i++) {
			triArea -= TriangleArea(p[i], p[j], pointP);
		}
		if (abs(triArea) < faceDistance) {
			nearestFace = f_it.handle();
			faceDistance = abs(triArea);
		}
	}
	printf("point: %lf, %lf, %lf\n", point.x, point.y, point.z);
	MyMesh::Point p[3];
	MyMesh::TexCoord2D t[3];
	double w[4] = {};
	int i = 0, j = 2;
	for (MyMesh::FaceHalfedgeIter fh_it = mesh->fh_begin(nearestFace); fh_it != mesh->fh_end(nearestFace); ++fh_it) {
		t[i] = mesh->texcoord2D(fh_it);
		p[i++] = mesh->point(mesh->to_vertex_handle(fh_it));
		//printf("FaceVertexI: %lf, %lf, %lf\n", mesh->point(fv_it)[0], mesh->point(fv_it)[1], mesh->point(fv_it)[2]);
	}
	//for (MyMesh::FaceVertexIter fv_it = mesh->fv_begin(nearestFace); fv_it != mesh->fv_end(nearestFace); ++fv_it) {
	//	t[i] = mesh->texcoord2D(fv_it);
	//	p[i++] = mesh->point(fv_it);
	//	printf("FaceVertexI: %lf, %lf, %lf\n", mesh->point(fv_it)[0], mesh->point(fv_it)[1], mesh->point(fv_it)[2]);
	//}
	//for (i = 0, j = 2; i < 3; j = i++) {
	//	int k = (i + 1) % 3;
	//	double tria = TriangleArea(p[i], p[j], pointP);
	//	w[k] = tria * 2 / (p[i] - p[j]).norm();
	//	//w[k] = (p[k] - pointP).norm();
	//	w[3] += w[k];
	//}
	//MyMesh::TexCoord2D strokePosition = t[0] * (w[0] / w[3]) + t[1] * (w[1] / w[3]) + t[2] * (w[2] / w[3]);
	p[2] -= p[0];
	p[1] -= p[0];
	pointP -= p[0];
	glm::vec3 b1(p[2][0], p[2][1], p[2][2]);
	glm::vec3 b2(p[1][0], p[1][1], p[1][2]);
	glm::vec3 pp(pointP[0], pointP[1], pointP[2]);
	glm::vec2 x = glm::inverse(glm::mat2(glm::dot(b1, b1), glm::dot(b1, b2), glm::dot(b2, b1), glm::dot(b2, b2))) * glm::vec2(glm::dot(b1, pp), glm::dot(b2, pp));
	MyMesh::TexCoord2D strokePosition = t[0] + (t[2] - t[0]) * x[0] + (t[1] - t[0]) * x[1];
	strokePointQueue.push(glm::vec3(strokePosition[0], strokePosition[1], 0));
	printf("faceDistance: %lf, UV: %lf, %lf\n", faceDistance, strokePosition[0], strokePosition[1]);
}

void ModelPart::EndDraw()
{
	state = ModelState::STATE_MODEL;
	if (strokeFBOTextureData)
		free(strokeFBOTextureData);
	strokeFBOTextureData = writeTextureToArray(strokeFBOColorTexture);
	strokeFBOTextureExpired = true;
}

void ModelPart::UpdateMeshBuffer()
{
	if (!mesh) {
		return;
	}
	//if (mesh) {
	//	glDeleteVertexArrays(1, &vao);
	//	glDeleteBuffers(1, &vbo);
	//	glDeleteBuffers(1, &ebo);
	//}
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//maxPointDist = 0;
	//vertexCount = mesh->n_vertices();
	//GLdouble *vertexData = new GLdouble[vertexCount * 8];
	//for (MyMesh::VertexIter v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it) {
	//	memcpy(vertexData + (v_it->idx() * 8), mesh->point(v_it.handle()).data(), sizeof(GLdouble) * 3);
	//	memcpy(vertexData + (v_it->idx() * 8 + 3), mesh->normal(v_it.handle()).data(), sizeof(GLdouble) * 3);
	//	vertexData[v_it->idx() * 8 + 6] = mesh->texcoord2D(v_it.handle()).data()[0];
	//	vertexData[v_it->idx() * 8 + 7] = mesh->texcoord2D(v_it.handle()).data()[1];
	//	for (int i = 0; i < 3; i++) {
	//		if (abs(mesh->point(v_it.handle())[i]) > maxPointDist) {
	//			maxPointDist = abs(mesh->point(v_it.handle())[i]);
	//		}
	//	}
	//}
	//glGenBuffers(1, &vbo);
	//glBindBuffer(GL_ARRAY_BUFFER, vbo);
	//glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(GLdouble) * 8, vertexData, GL_STATIC_DRAW);
	//glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, sizeof(GLdouble) * 8, 0);
	//glVertexAttribPointer(1, 3, GL_DOUBLE, GL_FALSE, sizeof(GLdouble) * 8, (GLvoid*)(sizeof(GLdouble) * 3));
	//glVertexAttribPointer(2, 2, GL_DOUBLE, GL_FALSE, sizeof(GLdouble) * 8, (GLvoid*)(sizeof(GLdouble) * 6));
	//glEnableVertexAttribArray(0);
	//glEnableVertexAttribArray(1);
	//glEnableVertexAttribArray(2);
	//delete[] vertexData;

	//faceCount = mesh->n_faces();
	//GLuint *vertexIndices = new GLuint[faceCount * 3];
	//for (MyMesh::FaceIter f_it = mesh->faces_begin(); f_it != mesh->faces_end(); ++f_it) {
	//	int i = 0;
	//	for (MyMesh::FaceVertexIter fv_it = mesh->fv_iter(f_it.handle()); fv_it.is_valid(); ++fv_it, ++i) {
	//		vertexIndices[f_it->idx() * 3 + i] = fv_it->idx();
	//	}
	//}
	//glGenBuffers(1, &ebo);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceCount * sizeof(GLuint) * 3, vertexIndices, GL_STATIC_DRAW);
	//delete[] vertexIndices;



	maxPointDist = 0;
	vertexCount = 0;
	faceCount = mesh->n_faces();
	GLdouble *vertexData = new GLdouble[faceCount * 3 * 8];
	GLuint *vertexIndices = new GLuint[faceCount * 3];
	for (MyMesh::FaceIter f_it = mesh->faces_begin(); f_it != mesh->faces_end(); ++f_it) {
		for (MyMesh::FaceHalfedgeIter fhe_it = mesh->fh_begin(f_it.handle()); fhe_it != mesh->fh_end(f_it.handle()); ++fhe_it) {
			memcpy(vertexData + (vertexCount * 8 + 0), mesh->point(mesh->to_vertex_handle(fhe_it)).data(), sizeof(GLdouble) * 3);
			memcpy(vertexData + (vertexCount * 8 + 3), mesh->normal(mesh->to_vertex_handle(fhe_it)).data(), sizeof(GLdouble) * 3);
			//vertexData[vertexCount * 8 + 6] = 0.5;
			//vertexData[vertexCount * 8 + 7] = 0.5;
			vertexData[vertexCount * 8 + 6] = mesh->texcoord2D(fhe_it.handle()).data()[0];
			vertexData[vertexCount * 8 + 7] = mesh->texcoord2D(fhe_it.handle()).data()[1];
			vertexIndices[vertexCount] = vertexCount;
			vertexCount++;
		}
	}
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(GLdouble) * 8, vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, sizeof(GLdouble) * 8, 0);
	glVertexAttribPointer(1, 3, GL_DOUBLE, GL_FALSE, sizeof(GLdouble) * 8, (GLvoid*)(sizeof(GLdouble) * 3));
	glVertexAttribPointer(2, 2, GL_DOUBLE, GL_FALSE, sizeof(GLdouble) * 8, (GLvoid*)(sizeof(GLdouble) * 6));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceCount * sizeof(GLuint) * 3, vertexIndices, GL_STATIC_DRAW);
	delete[] vertexData;
	delete[] vertexIndices;



	glBindVertexArray(0);
}

bool ModelPart::isPointOnMesh(glm::vec3 point)
{
	return false;
}

float ModelPart::meshPointDistance(glm::vec3 point)
{
	float minDistance = FLT_MAX;
	MyMesh::Point p(point.x, point.y, point.z);
	if (!mesh)
		return minDistance;
	for (MyMesh::VertexIter v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it) {
		float pointDistance = (mesh->point(v_it.handle()) - p).norm();
		if (pointDistance < minDistance)
			minDistance = pointDistance;
	}
	return minDistance;
}

void ModelPart::readUVEdge(std::vector<glm::vec3> &uvVector)
{
	uvVector.clear();
	if (!mesh)
		return;
	for (MyMesh::HalfedgeIter he_it = mesh->halfedges_begin(); he_it != mesh->halfedges_end(); ++he_it) {
		OpenMesh::Vec2d uv1 = mesh->texcoord2D(he_it);
		OpenMesh::Vec2d uv2 = mesh->texcoord2D(mesh->next_halfedge_handle(he_it));
		uvVector.push_back(glm::vec3(uv1.data()[0], uv1.data()[1], 0));
		uvVector.push_back(glm::vec3(uv2.data()[0], uv2.data()[1], 0));
	}
}

void ModelPart::readContourScreenPoint(std::vector<glm::vec3> &pointVector)
{
	pointVector = screenPoints;
}

std::vector<glm::vec3>& ModelPart::getContourScreenPoint()
{
	return screenPoints;
}

bool ModelPart::isComputableContour(std::vector<glm::vec3> contourPoints)
{
	// reference https://www.ptt.cc/bbs/MATLAB/M.1297675543.A.40E.html
	if (contourPoints.size() < 3)
		return false;
	for (int ai = 0, aj = contourPoints.size() - 1; ai < contourPoints.size(); aj = ai++) {
		double p1 = contourPoints[aj].y - contourPoints[ai].y;
		double p2 = contourPoints[ai].x - contourPoints[aj].x;
		double p3 = contourPoints[aj].x*contourPoints[ai].y - contourPoints[ai].x*contourPoints[aj].y;
		for (int bi = ai + 2, bj = ai + 1; bi < contourPoints.size(); bj = bi++) {
			if (bi == aj || bj == ai)
				continue;
			double q1 = contourPoints[bj].y - contourPoints[bi].y;
			double q2 = contourPoints[bi].x - contourPoints[bj].x;
			double q3 = contourPoints[bj].x*contourPoints[bi].y - contourPoints[bi].x*contourPoints[bj].y;

			bool sign1 = ((p1*contourPoints[bi].x + p2*contourPoints[bi].y + p3) < 0) ^ ((p1*contourPoints[bj].x + p2*contourPoints[bj].y + p3) < 0);
			bool sign2 = ((q1*contourPoints[ai].x + q2*contourPoints[ai].y + q3) < 0) ^ ((q1*contourPoints[aj].x + q2*contourPoints[aj].y + q3) < 0);

			if (sign1 && sign2)
				return false;
		}
	}
	return true;
}

void ModelPart::SetTexture(GLuint textureID)
{
	modelTexture = textureID;
	clearStroke = true;
	drawModelTexture = true;
}

void ModelPart::UpdateContourPoint(int pointID)
{
	if (pointID >= 0 && pointID < screenPoints.size()) {
		points[pointID] = glm::unProject(screenPoints[pointID] * glm::vec3(1, 1, 0.5) + glm::vec3(0, 0, 0.5), mvp, glm::mat4(1), glm::vec4(-1, -1, 2, 2));
	}
	else {

	}
}

MyMesh ModelPart::GetPlane()
{
	if (!planeMesh)
		return MyMesh();
	MyMesh mesh = *planeMesh;
	for (MyMesh::VertexIter vh = mesh.vertices_begin(); vh != mesh.vertices_end(); ++vh) {
		MyMesh::Point& p = mesh.point(vh);
		glm::vec4 np = mvp * glm::vec4(p[0], p[1], p[2], 1);
		mesh.point(vh) = MyMesh::Point(np.x / np.w, np.y / np.w, np.z / np.w);
	}
	return mesh;
}

void ModelPart::SetPlane(MyMesh mesh)
{
	glm::mat4 invMvp = glm::inverse(mvp);
	for (MyMesh::VertexIter vh = mesh.vertices_begin(); vh != mesh.vertices_end(); ++vh) {
		MyMesh::Point& p = mesh.point(vh);
		glm::vec4 np = invMvp * glm::vec4(p[0], p[1], p[2], 1);
		mesh.point(vh) = MyMesh::Point(np.x / np.w, np.y / np.w, np.z / np.w);
	}
	*planeMesh = mesh;
}
