
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
		GetInstance().scene = std::make_unique<Battery::Scene>(App::GetApplicationPointer()->window);
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

void ApplicationRenderer::BeginFrame() {
	Battery::Renderer2D::BeginScene(GetInstance().scene.get());
}

void ApplicationRenderer::EndFrame() {
	Battery::Renderer2D::EndScene();
}






void ApplicationRenderer::DrawLineWorkspace(const glm::vec2& point1, const glm::vec2& point2, float thickness, 
	const glm::vec4& color, float falloff) {
	glm::vec2 p1 = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(point1);
	glm::vec2 p2 = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(point2);
	float thick = Navigator::GetInstance()->ConvertWorkspaceToScreenDistance(thickness);
	Battery::Renderer2D::DrawLine(p1, p2, max(thick, 0.5f), color, falloff);
}

void ApplicationRenderer::DrawLineScreenspace(const glm::vec2& point1, const glm::vec2& point2, float thickness, 
	const glm::vec4& color, float falloff) {
	Battery::Renderer2D::DrawLine(point1, point2, thickness, color, falloff);
}

void ApplicationRenderer::DrawLineExport(const glm::vec2& point1, const glm::vec2& point2, float thickness,
	const glm::vec4& color, glm::vec2 min, glm::vec2 max, float width, float height) {

	glm::vec2 p1 = Battery::MathUtils::MapVector(point1, min, max, { 0, 0 }, { width, height });
	glm::vec2 p2 = Battery::MathUtils::MapVector(point2, min, max, { 0, 0 }, { width, height });
	float thick = Battery::MathUtils::MapFloat(thickness, 0, max.x - min.x, 0, width);

	Battery::Renderer2D::DrawLine(p1, p2, thick, color, EXPORT_FALLOFF);
}

void ApplicationRenderer::DrawCircleWorkspace(const glm::vec2& center, float radius, float thickness, 
	const glm::vec4& color, float falloff) {
	glm::vec2 c = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(center);
	float r     = Navigator::GetInstance()->ConvertWorkspaceToScreenDistance(radius);
	float t     = Navigator::GetInstance()->ConvertWorkspaceToScreenDistance(thickness);
	Battery::Renderer2D::DrawCircle(c, r, t, color, { 0, 0, 0, 0 }, falloff);
}

void ApplicationRenderer::DrawCircleScreenspace(const glm::vec2& center, float radius, float thickness,
	const glm::vec4& color, float falloff) {
	Battery::Renderer2D::DrawCircle(center, radius, thickness, color, { 0, 0, 0, 0 }, falloff);
}

void ApplicationRenderer::DrawCircleExport(const glm::vec2& center, float radius, float thickness,
	const glm::vec4& color, glm::vec2 min, glm::vec2 max, float width, float height) {

	glm::vec2 c = Battery::MathUtils::MapVector(center, min, max, { 0, 0 }, { width, height });
	float r	    = Battery::MathUtils::MapFloat(radius, 0, max.x - min.x, 0, width);
	float t     = Battery::MathUtils::MapFloat(thickness, 0, max.x - min.x, 0, width);

	Battery::Renderer2D::DrawCircle(c, r, t, color, { 0, 0, 0, 0 }, EXPORT_FALLOFF);
}

void ApplicationRenderer::DrawArcWorkspace(const glm::vec2& center, float radius, float startAngle, float endAngle, float thickness,
	const glm::vec4& color, float falloff) {
	glm::vec2 c = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(center);
	float r     = Navigator::GetInstance()->ConvertWorkspaceToScreenDistance(radius);
	float t     = Navigator::GetInstance()->ConvertWorkspaceToScreenDistance(thickness);
	Battery::Renderer2D::DrawArc(c, r, startAngle, endAngle, t, color, falloff);
}

void ApplicationRenderer::DrawArcScreenspace(const glm::vec2& center, float radius, float startAngle, float endAngle, float thickness,
	const glm::vec4& color, float falloff) {
	Battery::Renderer2D::DrawArc(center, radius, startAngle, endAngle, thickness, color, falloff);
}

void ApplicationRenderer::DrawArcExport(const glm::vec2& center, float radius, float startAngle, float endAngle, float thickness,
	const glm::vec4& color, glm::vec2 min, glm::vec2 max, float width, float height) {

	glm::vec2 c = Battery::MathUtils::MapVector(center, min, max, { 0, 0 }, { width, height });
	float r	    = Battery::MathUtils::MapFloat(radius, 0, max.x - min.x, 0, width);
	float t     = Battery::MathUtils::MapFloat(thickness, 0, max.x - min.x, 0, width);

	Battery::Renderer2D::DrawArc(c, r, startAngle, endAngle, t, color, EXPORT_FALLOFF);
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

void ApplicationRenderer::DrawGrid(bool infinite) {
	using namespace Battery;
	auto nav = Navigator::GetInstance();

	float thickness = GetInstance().gridLineWidth;
	float alpha = min(Navigator::GetInstance()->scale * GetInstance().gridAlphaFactor + GetInstance().gridAlphaOffset, GetInstance().gridAlphaMax);
	glm::vec4 color = glm::vec4(GetInstance().gridLineColor, GetInstance().gridLineColor, GetInstance().gridLineColor, alpha);

	int w = GetClientApplication()->window.GetWidth();
	int h = GetClientApplication()->window.GetHeight();

	float right = w;
	float left = 0;
	float top = h;
	float bottom = 0;

	if (!infinite) {	// Draw an A4-sheet
		glm::vec2 sheetSize = { 210, 297 };
		right  = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords( sheetSize / 2.f).x;
		left   = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(-sheetSize / 2.f).x;
		top    = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords( sheetSize / 2.f).y;
		bottom = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(-sheetSize / 2.f).y;

		for (float x = nav->panOffset.x + w / 2; x < right; x += nav->scale * nav->snapSize) {
			Renderer2D::DrawPrimitiveLine({ x, bottom }, { x, top }, thickness, color);
		}
		for (float x = nav->panOffset.x + w / 2 - nav->scale * nav->snapSize; x > left; x -= nav->scale * nav->snapSize) {
			Renderer2D::DrawPrimitiveLine({ x, bottom }, { x, top }, thickness, color);
		}
		for (float y = nav->panOffset.y + h / 2; y < top; y += nav->scale * nav->snapSize) {
			Renderer2D::DrawPrimitiveLine({ left, y }, { right, y }, thickness, color);
		}
		for (float y = nav->panOffset.y + h / 2 - nav->scale * nav->snapSize; y > bottom; y -= nav->scale * nav->snapSize) {
			Renderer2D::DrawPrimitiveLine({ left, y }, { right, y }, thickness, color);
		}
	}
	else {

		for (float x = nav->panOffset.x + w / 2; x < right; x += nav->scale * nav->snapSize) {
			Renderer2D::DrawPrimitiveLine({ x, bottom }, { x, top }, thickness, color);
		}
		for (float x = nav->panOffset.x + w / 2; x > left; x -= nav->scale * nav->snapSize) {
			Renderer2D::DrawPrimitiveLine({ x, bottom }, { x, top }, thickness, color);
		}
		for (float y = nav->panOffset.y + h / 2; y < top; y += nav->scale * nav->snapSize) {
			Renderer2D::DrawPrimitiveLine({ left, y }, { right, y }, thickness, color);
		}
		for (float y = nav->panOffset.y + h / 2; y > bottom; y -= nav->scale * nav->snapSize) {
			Renderer2D::DrawPrimitiveLine({ left, y }, { right, y }, thickness, color);
		}
	}

	if (!infinite) {	// Draw sheet outline
		Renderer2D::DrawPrimitiveLine({ left,  bottom }, { right, bottom }, thickness * 2, color);
		Renderer2D::DrawPrimitiveLine({ right, bottom }, { right, top }, thickness * 2, color);
		Renderer2D::DrawPrimitiveLine({ right, top},     { left,  top }, thickness * 2, color);
		Renderer2D::DrawPrimitiveLine({ left,  top },    { left, bottom }, thickness * 2, color);
	}
}
