#include "DrawPanel.h"
#include "ShaderUtility.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

DrawPanel::DrawPanel()
{
	zNear = 0.1;
	zFar = 100;
	SetView(ViewDirection::ResetView);
	points.clear();
	ModelPart::modelMatrix = glm::mat4(1.0f);
	ModelPart::viewMatrix = glm::translate(glm::mat4(1.0f), transform);
	ModelPart::projectionMatrix = glm::mat4(1.0f);
	operateMode = OperateMode::PointMode;
	selectPart = -1;
}

DrawPanel::~DrawPanel()
{
}

void DrawPanel::Initialize()
{
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_POINT_SMOOTH);
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	ModelPart::InitProgram();
}

void DrawPanel::Reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	ModelPart::projectionMatrix = glm::perspective(glm::radians(45.0f), (float)width / (float)height, zNear, zFar);
}

void DrawPanel::Display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (int i = 0; i < parts.size(); i++) {
		parts[i]->Render();
	}
	if (selectPart != -1)
		parts[selectPart]->RenderLine();
}

void DrawPanel::MouseDown(int x, int y, int button)
{
	if (button == 0) {
		if (operateMode == OperateMode::PointMode) {
			glm::vec3 screenPos(x, height - y, 0);
			BindGL();
			glReadBuffer(GL_FRONT);
			glReadPixels(screenPos.x, screenPos.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &screenPos.z);
			ReleaseGL();
			if (screenPos.z == 1) {
				selectPart = -1;
			}
			else {
				glm::vec3 worldPos = glm::unProject(screenPos, ModelPart::viewMatrix * ModelPart::modelMatrix, ModelPart::projectionMatrix, glm::vec4(0, 0, width, height));
				int nearestPart = -1;
				float nearestDistance = FLT_MAX;
				for (int i = 0; i < parts.size(); i++) {
					float partDistance = parts[i]->meshPointDistance(worldPos);
					if (partDistance < nearestDistance) {
						nearestPart = i;
						nearestDistance = partDistance;
					}
				}
				selectPart = nearestPart;
			}
		}
		else if (operateMode == OperateMode::CreateMode) {
			parts.push_back(new ModelPart());
			glm::vec3 screenPos(x, height - y, 0);
			BindGL();
			parts.back()->CreateFrameBuffer(width, height);
			glReadBuffer(GL_FRONT);
			glReadPixels(screenPos.x, screenPos.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &zDraw);
			printf("bzDraw %lf\n", zDraw);
			if (zDraw == 1) {
				glm::vec4 p = ModelPart::projectionMatrix * ModelPart::viewMatrix * ModelPart::modelMatrix * glm::vec4(0, 0, 0, 1);
				zDraw = (p.z / p.w + 1) / 2;
			}
			printf("azDraw %lf\n", zDraw);
			//printf("P: %f, %f, %f, %f\n", p.x, p.y, p.z, p.w);
			//printf("screenPos.z: %f, %f, %f\n", screenPos.x, screenPos.y, screenPos.z);
			ReleaseGL();
			screenPos.z = zDraw;

			//screenPos.z = (zFar + transform.z) / (zFar - zNear);
			glm::vec3 worldPos = glm::unProject(screenPos, ModelPart::viewMatrix * ModelPart::modelMatrix, ModelPart::projectionMatrix, glm::vec4(0, 0, width, height));
			parts.back()->AddPoint(worldPos);

			//printf("screenPos.z: %f, %f, %f\n", screenPos.x, screenPos.y, screenPos.z);
			//printf("worldPos.z: %f, %f, %f\n", worldPos.x, worldPos.y, worldPos.z);

			//printf("transform.z: %f\n", transform.z);
			selectPart = parts.size() - 1;
		}
		else if (operateMode == OperateMode::DrawMode) {

		}
		else if (operateMode == OperateMode::DeformationMode) {

		}
	}
	previousMousePosition.x = x;
	previousMousePosition.y = y;
}

void DrawPanel::MouseUp(int x, int y, int button)
{
	if (button == 0 && parts.size() > 0) {
		if (operateMode == OperateMode::PointMode) {

		}
		else if (operateMode == OperateMode::CreateMode) {
			if (parts.size() > 0) {
				BindGL();
				parts.back()->CreateMesh();
				parts.back()->UpdateMeshBuffer();
				ReleaseGL();
			}
		}
		else if (operateMode == OperateMode::DrawMode) {

		}
		else if (operateMode == OperateMode::DeformationMode) {

		}
	}
}

void DrawPanel::MouseMove(int x, int y)
{
	if (isLMBDown) {
		if (operateMode == OperateMode::PointMode) {

		}
		else if (operateMode == OperateMode::CreateMode) {
			glm::vec3 screenPos(x, height - y, 0);
			//BindGL();
			//glReadBuffer(GL_FRONT);
			//glReadPixels(screenPos.x, screenPos.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &screenPos.z);
			//ReleaseGL();
			//screenPos.z = 1.0 - (zFar + transform.z) / (zFar - zNear);
			//glm::vec4 p = ModelPart::projectionMatrix * ModelPart::viewMatrix * ModelPart::modelMatrix * glm::vec4(0, 0, 0, 1);
			//screenPos.z = (p.z / p.w + 1) / 2;
			screenPos.z = zDraw;
			glm::vec3 worldPos = glm::unProject(screenPos, ModelPart::viewMatrix * ModelPart::modelMatrix, ModelPart::projectionMatrix, glm::vec4(0, 0, width, height));
			parts.back()->AddPoint(worldPos);
			//AddPoint(worldPos.x, worldPos.y, worldPos.z);
		}
		else if (operateMode == OperateMode::DrawMode) {

		}
		else if (operateMode == OperateMode::DeformationMode) {

		}
	}
	else if (isRMBDown) {
		//glm::mat4 model = glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1, 0, 0));
		//model = glm::rotate(model, rotation.y, glm::vec3(0, 1, 0));
		//glm::mat4 view = glm::translate(glm::mat4(1.0f), transform);
		rotation.x += glm::radians((float)(y - previousMousePosition.y));
		rotation.y += glm::radians((float)(x - previousMousePosition.x));
		ModelPart::modelMatrix = glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1, 0, 0));
		ModelPart::modelMatrix = glm::rotate(ModelPart::modelMatrix, rotation.y, glm::vec3(0, 1, 0));
	}
	if (isMMBDown) {
		transform.x += (x - previousMousePosition.x) * glm::abs(transform.z) / 500;
		transform.y -= (y - previousMousePosition.y) * glm::abs(transform.z) / 500;
		ModelPart::viewMatrix = glm::translate(glm::mat4(1.0f), transform);
	}
	previousMousePosition.x = x;
	previousMousePosition.y = y;
}

void DrawPanel::MouseWheel(int x, int y, int delta)
{
	if (isRMBDown) {
		if (delta < 0) {
			zoom += 0.1;
		}
		else {
			zoom -= 0.1;
		}
	}
	else if (isMMBDown) {
		if (delta < 0) {
			zoom += 10;
		}
		else {
			zoom -= 10;
		}
	}
	else {
		if (delta < 0) {
			zoom += 1;
		}
		else {
			zoom -= 1;
		}
	}
	transform.z = -pow(2, zoom);
	ModelPart::viewMatrix = glm::translate(glm::mat4(1.0f), transform);
}

void DrawPanel::RemovePart(int id)
{
	if (id >= 0 && id < parts.size()) {
		delete parts[id];
		parts.erase(parts.begin() + id);
	}
}

void DrawPanel::ReCreatePart(int id)
{
	if (id >= 0 && id < parts.size()) {
		BindGL();
		parts[id]->CreateMesh();
		parts[id]->UpdateMeshBuffer();
		ReleaseGL();
	}
}

void DrawPanel::ExtractionPart(int id, float s)
{
	if (id >= 0 && id < parts.size()) {
		BindGL();
		parts[id]->ExtractionMesh(s);
		parts[id]->UpdateMeshBuffer();
		ReleaseGL();
	}
}

void DrawPanel::SmoothPart(int id, int step)
{
	if (id >= 0 && id < parts.size()) {
		BindGL();
		parts[id]->SmoothMesh(step);
		parts[id]->UpdateMeshBuffer();
		ReleaseGL();
	}
}

void DrawPanel::SetView(ViewDirection viewDirection)
{
	switch (viewDirection) {
	case FrontView:
		rotation = glm::vec3(0, 0, 0);
		break;
	case BackView:
		rotation = glm::vec3(0, glm::pi<float>(), 0);
		break;
	case TopView:
		rotation = glm::vec3(glm::pi<float>() / 2, 0, 0);
		break;
	case BottomView:
		rotation = glm::vec3(-glm::pi<float>() / 2, 0, 0);
		break;
	case RightView:
		rotation = glm::vec3(0, -glm::pi<float>() / 2, 0);
		break;
	case LeftView:
		rotation = glm::vec3(0, glm::pi<float>() / 2, 0);
		break;
	case FTRView:
		rotation = glm::vec3(glm::pi<float>() / 4, -glm::pi<float>() / 4, 0);
		break;
	case BBLView:
		rotation = glm::vec3(-glm::pi<float>() / 4, glm::pi<float>() / 4, 0);
		break;
	case ResetView:
		zoom = 0;
		transform = glm::vec3(0, 0, -pow(2, zoom));
		rotation = glm::vec3(0, 0, 0);
		break;
	default:
		break;
	}
	ModelPart::modelMatrix = glm::mat4(1.0f);
	ModelPart::viewMatrix = glm::translate(glm::mat4(1.0f), transform);
}

void DrawPanel::ExportOBJ(const char* fileName)
{
	ModelPart::SavePartsToOBJ(parts, fileName);
}

void DrawPanel::LoadModel(const char* fileName)
{
	parts.push_back(new ModelPart());
	parts.back()->ReadMesh(fileName);
	BindGL();
	parts.back()->UpdateMeshBuffer();
	ReleaseGL();
	selectPart = parts.size() - 1;
}
