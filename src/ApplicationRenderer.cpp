
#include "pch.h"
#include "ApplicationRenderer.h"
#include "Battery/Battery.h"
#include "Application.h"
#include "Navigator.h"
#include "config.h"

ApplicationRenderer::ApplicationRenderer() {

}

ApplicationRenderer::~ApplicationRenderer() {
	if (GetInstance().scene) {
		LOG_ERROR(__FUNCTION__"(): The Renderer was not unloaded! You must unload it before terminating!");
		GetInstance().scene.release();
	}
}

void ApplicationRenderer::Load() {
	if (!GetInstance().scene) {
		GetInstance().scene = std::make_unique<Battery::Scene>(&App::GetApplicationPointer()->window);
	}
	else {
		throw Battery::Exception(__FUNCTION__"(): Can't load renderer: Is already loaded!");
	}
}

void ApplicationRenderer::Unload() {
	if (GetInstance().scene) {
		GetInstance().scene.release();
	}
	else {
		throw Battery::Exception(__FUNCTION__"(): Can't unload renderer: Is not loaded!");
	}
}

void ApplicationRenderer::BeginFrame(bool background) {
	Battery::Renderer2D::BeginScene(GetInstance().scene.get());

	if (background)
		Battery::Renderer2D::DrawBackground(BACKGROUND_COLOR);
}

void ApplicationRenderer::EndFrame() {
	Battery::Renderer2D::EndScene();
}






void ApplicationRenderer::DrawLineWorkspace(const glm::vec2& point1, const glm::vec2& point2, float thickness, 
	const glm::vec4& color, float falloff) {
	glm::vec2 p1 = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(point1);
	glm::vec2 p2 = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(point2);
	float thick = Navigator::GetInstance()->ConvertWorkspaceToScreenDistance(thickness);
	Battery::Renderer2D::DrawLine(p1, p2, thick, color, falloff);
}

void ApplicationRenderer::DrawLineScreenspace(const glm::vec2& point1, const glm::vec2& point2, float thickness, 
	const glm::vec4& color, float falloff) {
	Battery::Renderer2D::DrawLine(point1, point2, thickness, color, falloff);
}

void ApplicationRenderer::DrawRectangleWorkspace(const glm::vec2& point1, const glm::vec2& point2, float outlineThickness,
	const glm::vec4& outlineColor, const glm::vec4& fillColor, float falloff) {
	glm::vec2 p1 = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(point1);
	glm::vec2 p2 = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(point2);
	Battery::Renderer2D::DrawRectangle(p1, p2, outlineThickness, outlineColor, fillColor, falloff);
}

void ApplicationRenderer::DrawRectangleScreenspace(const glm::vec2& point1, const glm::vec2& point2, float outlineThickness,
	const glm::vec4& outlineColor, const glm::vec4& fillColor, float falloff) {
	Battery::Renderer2D::DrawRectangle(point1, point2, outlineThickness, outlineColor, fillColor, falloff);
}





void ApplicationRenderer::DrawSelectionBoxInfillRectangle(const glm::vec2& point1, const glm::vec2& point2) {
	ApplicationRenderer::DrawRectangleWorkspace(point1, point2, 0, { 0, 0, 0, 0 }, GetInstance().selectionBoxFillColor, 0);
}

void ApplicationRenderer::DrawSelectionBoxOutlineRectangle(const glm::vec2& point1, const glm::vec2& point2) {
	ApplicationRenderer::DrawRectangleWorkspace(point1, point2, 
		GetInstance().selectionBoxOutlineThickness, GetInstance().selectionBoxOutlineColor, { 0, 0, 0, 0 }, 0);
}

void ApplicationRenderer::DrawPreviewPoint(const glm::vec2& position) {
	glm::vec2 p1 = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(position);
	ApplicationRenderer::DrawRectangleScreenspace(
		p1 - glm::vec2(GetInstance().previewPointSize / 2, GetInstance().previewPointSize / 2),
		p1 + glm::vec2(GetInstance().previewPointSize / 2, GetInstance().previewPointSize / 2),
		1, { 0, 0, 0, 255 }, { 255, 255, 255, 255 }, 0);
}

void ApplicationRenderer::DrawGrid() {
	using namespace Battery;
	auto nav = Navigator::GetInstance();

	float thickness = GetInstance().gridLineWidth;
	float alpha = min(Navigator::GetInstance()->scale * GetInstance().gridAlphaFactor + GetInstance().gridAlphaOffset, GetInstance().gridAlphaMax);
	glm::vec4 color = glm::vec4(GetInstance().gridLineColor, GetInstance().gridLineColor, GetInstance().gridLineColor, alpha);

	int w = GetClientApplication()->window.GetWidth();
	int h = GetClientApplication()->window.GetHeight();

	// Sub grid lines
	//if (scale * snapSize > 3) {
	for (float x = nav->panOffset.x + w / 2; x < w; x += nav->scale * nav->snapSize) {
		Renderer2D::DrawPrimitiveLine({ x, 0 }, { x, h }, thickness, color);
	}
	for (float x = nav->panOffset.x + w / 2; x > 0; x -= nav->scale * nav->snapSize) {
		Renderer2D::DrawPrimitiveLine({ x, 0 }, { x, h }, thickness, color);
	}
	for (float y = nav->panOffset.y + h / 2; y < h; y += nav->scale * nav->snapSize) {
		Renderer2D::DrawPrimitiveLine({ 0, y }, { w, y }, thickness, color);
	}
	for (float y = nav->panOffset.y + h / 2; y > 0; y -= nav->scale * nav->snapSize) {
		Renderer2D::DrawPrimitiveLine({ 0, y }, { w, y }, thickness, color);
	}
	//}
}
