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
	drawSolid.textureHandle = loadTextureFromFilePNG("grid32.png");
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
	std::deque<MyMesh::VertexHandle> verts;
	MyMesh::VertexHandle vh[3];
	for (int i = 0; i < parts.size(); i++) {
		if (!parts[i]->mesh)
			continue;
		verts.clear();
		for (MyMesh::VertexIter v_it = parts[i]->mesh->vertices_begin(); v_it != parts[i]->mesh->vertices_end(); ++v_it) {
			verts.push_back(mesh.add_vertex(parts[i]->mesh->point(v_it)));
		}
		for (MyMesh::FaceIter f_it = parts[i]->mesh->faces_begin(); f_it != parts[i]->mesh->faces_end(); ++f_it) {
			MyMesh::FaceVertexIter fv_it = parts[i]->mesh->fv_begin(f_it);
			for (int j = 0; fv_it != parts[i]->mesh->fv_end(f_it); j++, ++fv_it) {
				vh[j] = verts[fv_it->idx()];
			}
			mesh.add_face(vh, 3);
		}
	}
	OpenMesh::IO::write_mesh(mesh, fileName);
}

ModelPart::ModelPart()
{
	mesh = NULL;
	mvp = projectionMatrix * viewMatrix * modelMatrix;
	state = ModelState::STATE_NONE;
}

ModelPart::~ModelPart()
{
}

void ModelPart::Render()
{
	if (state == ModelState::STATE_MODEL) {
		RenderModel();
		//RenderLine();
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
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if (!pointQueue.empty()) {
		glBindFramebuffer(GL_FRAMEBUFFER, strokeFBO);
		glUseProgram(drawStroke.program);
		glUniform4fv(drawStroke.colorLocation, 1, glm::value_ptr(strokeColor));
		glUniform1f(drawStroke.strokeSizeLocation, strokeSize);
		glUniformMatrix4fv(drawStroke.modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		glUniformMatrix4fv(drawStroke.viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(drawStroke.projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, strokeTextureHandle);
		glUniform1i(drawStroke.strokeTextureLocation, 0);
		glEnableVertexAttribArray(0);
		glm::vec3 *pointTemp = new glm::vec3[pointQueue.size()];
		int i;
		for (i = 0; !pointQueue.empty(); i++) {
			pointTemp[i] = pointQueue.front();
			pointQueue.pop();
		}
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, pointTemp);
		glDrawArrays(GL_POINTS, 0, i);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	glClear(GL_DEPTH_BUFFER_BIT);
	glUseProgram(drawTexture.program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, strokeFBOColorTexture);
	glUniform1i(drawTexture.textureLocation, 0);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void ModelPart::RenderLine()
{
	//glBindVertexArray(0);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	////glClear(GL_DEPTH_BUFFER_BIT);
	////glUseProgram(drawColor.program);
	////glUniform4fv(drawColor.colorLocation, 1, glm::value_ptr(glm::vec4(1, 0, 0, 1)));
	////glEnableVertexAttribArray(0);
	////glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, &points[0]);
	////glDrawArrays(GL_LINE_STRIP, 0, points.size());

	//glUseProgram(drawSolid.program);
	//glUniformMatrix4fv(drawSolid.modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	//glUniformMatrix4fv(drawSolid.viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	//glUniformMatrix4fv(drawSolid.projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	//glUniform4fv(drawSolid.colorLocation, 1, glm::value_ptr(strokeColor));
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, &points[0]);
	//glDrawArrays(GL_LINE_STRIP, 0, points.size());


	if (!mesh) {
		return;
	}
	glLineWidth(3);
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
	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
	glBindTexture(GL_TEXTURE_2D, drawSolid.textureHandle);
	glBindVertexArray(vao);
	glUniform4fv(drawSolid.colorLocation, 1, glm::value_ptr(glm::vec4(0.5, 0.5, 0.5, 1.0)));
	glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);
	glUniform1i(drawSolid.useTextureLocation, 0);
	//if (modelRenderLine) {
	//	glUniform4fv(drawSolid.colorLocation, 1, glm::value_ptr(glm::vec4(0.2, 0.2, 0.2, 1.0)));
	//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//	glEnable(GL_POLYGON_OFFSET_LINE);
	//	glPolygonOffset(-0.5, -0.5);
	//	glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);
	//	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//}
	//if (modelRenderPoint) {
	//	glDisable(GL_CULL_FACE);
	//	glPointSize(5);
	//	glUniform4fv(drawSolid.colorLocation, 1, glm::value_ptr(glm::vec4(0, 1, 0, 1.0)));
	//	//glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	//	glEnable(GL_POLYGON_OFFSET_POINT);
	//	glPolygonOffset(-0.6, -0.6);
	//	glDrawArrays(GL_POINTS, 0, vertexCount);
	//	//glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);
	//	glEnable(GL_CULL_FACE);
	//	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//}
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
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
}

void ModelPart::CreateMesh()
{
	if (!isComputableContour(screenPoints))
		return;
	printf("CreateMesh\n");
	//mesh = MyMesh::CreateFace(points);
	//mesh = Triangulation::CreateFace(&points[0], points.size(), triAspect, triSize);
	mesh = Triangulation::CreateFace(points, mvp, triAspect, triSize);
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

void ModelPart::AddPoint(float x, float y, float z)
{
	AddPoint(glm::vec3(x, y, z));
}

void ModelPart::AddPoint(glm::vec3 point)
{
	glm::vec4 screenPoint = mvp * glm::vec4(point, 1);
	screenPoint /= screenPoint.w;
	if (points.empty()) {
		pointQueue.push(point);
		points.push_back(point);
		screenPoints.push_back(screenPoint);
	}
	else {
		float screenLength = glm::length(glm::vec3(screenPoint) - screenPoints.back());
		if (screenLength > contourInterval) {
			glm::vec3 vector = point - points.back();
			float lineLength = glm::length(vector);
			// add interpolation point
			float interval = lineLength / (int)(screenLength / contourInterval);
			vector = glm::normalize(vector) * interval;
			for (int i = 1; i < (int)(screenLength / contourInterval); i ++) {
				points.push_back(points.back() + vector);
			}
			// add point
			points.push_back(point);
			screenPoints.push_back(screenPoint);
			// add interpolation stroke
			interval = lineLength / (int)(screenLength / strokeInterval);
			vector = glm::normalize(vector) * -interval;
			for (int i = 0; i < (int)(screenLength / strokeInterval); i ++) {
				pointQueue.push(points.back() + vector * float(i));
			}
		}
	}
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

	maxPointDist = 0;
	vertexCount = mesh->n_vertices();
	GLdouble *vertexData = new GLdouble[vertexCount * 8];
	for (MyMesh::VertexIter v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it) {
		memcpy(vertexData + (v_it->idx() * 8), mesh->point(v_it.handle()).data(), sizeof(GLdouble) * 3);
		memcpy(vertexData + (v_it->idx() * 8 + 3), mesh->normal(v_it.handle()).data(), sizeof(GLdouble) * 3);
		vertexData[v_it->idx() * 8 + 6] = mesh->texcoord2D(v_it.handle()).data()[0];
		vertexData[v_it->idx() * 8 + 7] = mesh->texcoord2D(v_it.handle()).data()[1];
		for (int i = 0; i < 3; i++) {
			if (abs(mesh->point(v_it.handle())[i]) > maxPointDist) {
				maxPointDist = abs(mesh->point(v_it.handle())[i]);
			}
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
	delete[] vertexData;

	faceCount = mesh->n_faces();
	GLuint *vertexIndices = new GLuint[faceCount * 3];
	for (MyMesh::FaceIter f_it = mesh->faces_begin(); f_it != mesh->faces_end(); ++f_it) {
		int i = 0;
		for (MyMesh::FaceVertexIter fv_it = mesh->fv_iter(f_it.handle()); fv_it.is_valid(); ++fv_it, ++i) {
			vertexIndices[f_it->idx() * 3 + i] = fv_it->idx();
		}
	}
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceCount * sizeof(GLuint) * 3, vertexIndices, GL_STATIC_DRAW);
	delete[] vertexIndices;



	//maxPointDist = 0;
	//vertexCount = 0;
	//faceCount = mesh->n_faces();
	//GLdouble *vertexData = new GLdouble[faceCount * 3 * 8];
	//GLuint *vertexIndices = new GLuint[faceCount * 3];
	//for (MyMesh::FaceIter f_it = mesh->faces_begin(); f_it != mesh->faces_end(); ++f_it) {
	//	for (MyMesh::FaceHalfedgeIter fhe_it = mesh->fh_begin(f_it.handle()); fhe_it != mesh->fh_end(f_it.handle()); ++fhe_it) {
	//		memcpy(vertexData + (vertexCount * 8 + 0), mesh->point(mesh->to_vertex_handle(fhe_it)).data(), sizeof(GLdouble) * 3);
	//		memcpy(vertexData + (vertexCount * 8 + 3), mesh->normal(mesh->to_vertex_handle(fhe_it)).data(), sizeof(GLdouble) * 3);
	//		//vertexData[vertexCount * 8 + 6] = 0.5;
	//		//vertexData[vertexCount * 8 + 7] = 0.5;
	//		vertexData[vertexCount * 8 + 6] = mesh->texcoord2D(fhe_it.handle()).data()[0];
	//		vertexData[vertexCount * 8 + 7] = mesh->texcoord2D(fhe_it.handle()).data()[1];
	//		vertexIndices[vertexCount] = vertexCount;
	//		vertexCount++;
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
	//glGenBuffers(1, &ebo);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceCount * sizeof(GLuint) * 3, vertexIndices, GL_STATIC_DRAW);
	//delete[] vertexData;
	//delete[] vertexIndices;



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
	for (MyMesh::EdgeIter e_it = mesh->edges_begin(); e_it != mesh->edges_end(); ++e_it) {
		MyMesh::HalfedgeHandle he = mesh->halfedge_handle(e_it, 0);
		OpenMesh::Vec2d uv1 = mesh->texcoord2D(mesh->from_vertex_handle(he));
		OpenMesh::Vec2d uv2 = mesh->texcoord2D(mesh->to_vertex_handle(he));
		uvVector.push_back(glm::vec3(uv1.data()[0], uv1.data()[1], 0));
		uvVector.push_back(glm::vec3(uv2.data()[0], uv2.data()[1], 0));
	}
}

void ModelPart::readContourScreenPoint(std::vector<glm::vec3> &pointVector)
{
	pointVector = screenPoints;
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
