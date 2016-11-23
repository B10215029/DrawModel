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
float ModelPart::pointInterval(0.005f);
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
	strokeTextureHandle = loadTextureFromFilePNG("./stroke/stroke1.png");
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
		RenderStroke();
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
	glLineWidth(3);
	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glClear(GL_DEPTH_BUFFER_BIT);
	//glUseProgram(drawColor.program);
	//glUniform4fv(drawColor.colorLocation, 1, glm::value_ptr(glm::vec4(1, 0, 0, 1)));
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, &points[0]);
	//glDrawArrays(GL_LINE_STRIP, 0, points.size());

	glUseProgram(drawSolid.program);
	glUniformMatrix4fv(drawSolid.modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(drawSolid.viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(drawSolid.projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniform4fv(drawSolid.colorLocation, 1, glm::value_ptr(strokeColor));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, &points[0]);
	glDrawArrays(GL_LINE_STRIP, 0, points.size());
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
	glBindVertexArray(vao);
	glUniform4fv(drawSolid.colorLocation, 1, glm::value_ptr(glm::vec4(0.5, 0.5, 0.5, 1.0)));
	glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);
	if (modelRenderLine) {
		glUniform4fv(drawSolid.colorLocation, 1, glm::value_ptr(glm::vec4(0.2, 0.2, 0.2, 1.0)));
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glEnable(GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(-0.5, -0.5);
		glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	if (modelRenderPoint) {
		glDisable(GL_CULL_FACE);
		glPointSize(5);
		glUniform4fv(drawSolid.colorLocation, 1, glm::value_ptr(glm::vec4(0, 1, 0, 1.0)));
		//glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		glEnable(GL_POLYGON_OFFSET_POINT);
		glPolygonOffset(-0.6, -0.6);
		glDrawArrays(GL_POINTS, 0, vertexCount);
		//glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, 0);
		glEnable(GL_CULL_FACE);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
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
	printf("CreateMesh\n");
	//mesh = MyMesh::CreateFace(points);
	mesh = Triangulation::CreateFace(&points[0], points.size(), triAspect, triSize);
	mesh->Extrude(extrudeThickness, extrudeDivisions, extrudeOffset, swellSize, swellPower);
	state = ModelState::STATE_MODEL;
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
	
	if (points.empty()) {
		pointQueue.push(point);
		points.push_back(point);
		//printf("add point%d: %f, %f, %f\n", points.size(), point.x, point.y, point.z);
	}
	else {
		glm::vec3 vector = point - points.back();
		float lineLength = glm::length(vector);
		vector = glm::normalize(vector);
		for (float i = pointInterval; i < lineLength; i += pointInterval) {
			pointQueue.push(points.back() + vector * i);
		}
		if (lineLength > pointInterval) {
			points.push_back(point);
			//printf("add point%d: %f, %f, %f\n", points.size(), point.x, point.y, point.z);
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
	GLdouble *vertexData = new GLdouble[vertexCount * 6];
	for (MyMesh::VertexIter v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it) {
		memcpy(vertexData + (v_it->idx() * 6), mesh->point(v_it.handle()).data(), sizeof(GLdouble) * 3);
		memcpy(vertexData + (v_it->idx() * 6 + 3), mesh->normal(v_it.handle()).data(), sizeof(GLdouble) * 3);
		for (int i = 0; i < 3; i++) {
			if (abs(mesh->point(v_it.handle())[i]) > maxPointDist) {
				maxPointDist = abs(mesh->point(v_it.handle())[i]);
			}
		}
	}
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(GLdouble) * 6, vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, sizeof(GLdouble) * 6, 0);
	glVertexAttribPointer(1, 3, GL_DOUBLE, GL_FALSE, sizeof(GLdouble) * 6, (GLvoid*)(sizeof(GLdouble) * 3));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
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

	glBindVertexArray(0);
}
