//
//#include "battery/core/all.hpp"
//#include "App.hpp"
//#include "ApplicationRenderer.h"
//#include "Navigator.h"
//#include "config.h"
//
//
//
//
//
//
//void ApplicationRenderer::DrawLineWorkspace(const ImVec2& point1, const ImVec2& point2, float thickness,
//	const ImColor& color) {
//	ImVec2 p1 = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(point1);
//	ImVec2 p2 = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(point2);
//	float thick = Navigator::GetInstance()->ConvertWorkspaceToScreenDistance(thickness);
//	//::DrawLine(p1, p2, std::max(thick, 0.5f), color, falloff);
//}
//
//void ApplicationRenderer::DrawLineScreenspace(const ImVec2& point1, const ImVec2& point2, float thickness,
//	const ImColor& color) {
//	//Battery::Renderer2D::DrawLine(point1, point2, thickness, color, falloff);
//}
//
//void ApplicationRenderer::DrawLineExport(const ImVec2& point1, const ImVec2& point2, float thickness,
//	const ImVec4& color, ImVec2 min, ImVec2 max, float width, float height) {
//
//	ImVec2 p1 = b::map(point1, min, max, { 0, 0 }, { width, height });
//	ImVec2 p2 = b::map(point2, min, max, { 0, 0 }, { width, height });
//	float thick = b::map(thickness, 0, max.x - min.x, 0, width);
//
//	//Battery::Renderer2D::DrawLine(p1, p2, thick, color, EXPORT_FALLOFF);
//}
//
//void ApplicationRenderer::DrawCircleWorkspace(const ImVec2& center, float radius, float thickness,
//	const ImColor& color) {
//	ImVec2 c = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(center);
//	float r     = Navigator::GetInstance()->ConvertWorkspaceToScreenDistance(radius);
//	float t     = Navigator::GetInstance()->ConvertWorkspaceToScreenDistance(thickness);
//	//Battery::Renderer2D::DrawCircle(c, r, t, color, { 0, 0, 0, 0 }, falloff);
//}
//
//void ApplicationRenderer::DrawCircleScreenspace(const ImVec2& center, float radius, float thickness,
//	const ImColor& color) {
//	//Battery::Renderer2D::DrawCircle(center, radius, thickness, color, { 0, 0, 0, 0 }, falloff);
//}
//
//void ApplicationRenderer::DrawCircleExport(const ImVec2& center, float radius, float thickness,
//	const ImVec4& color, ImVec2 min, ImVec2 max, float width, float height) {
//
//	ImVec2 c = b::map(center, min, max, { 0, 0 }, { width, height });
//	float r	    = b::map(radius, 0, max.x - min.x, 0, width);
//	float t     = b::map(thickness, 0, max.x - min.x, 0, width);
//
//	//Battery::Renderer2D::DrawCircle(c, r, t, color, { 0, 0, 0, 0 }, EXPORT_FALLOFF);
//}
//
//void ApplicationRenderer::DrawArcWorkspace(const ImVec2& center, float radius, float startAngle, float endAngle, float thickness,
//	const ImColor& color) {
//	ImVec2 c = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(center);
//	float r     = Navigator::GetInstance()->ConvertWorkspaceToScreenDistance(radius);
//	float t     = Navigator::GetInstance()->ConvertWorkspaceToScreenDistance(thickness);
//	//Battery::Renderer2D::DrawArc(c, r, startAngle, endAngle, t, color, falloff);
//}
//
//void ApplicationRenderer::DrawArcScreenspace(const ImVec2& center, float radius, float startAngle, float endAngle, float thickness,
//	const ImColor& color) {
//	//Battery::Renderer2D::DrawArc(center, radius, startAngle, endAngle, thickness, color, falloff);
//}
//
//void ApplicationRenderer::DrawArcExport(const ImVec2& center, float radius, float startAngle, float endAngle, float thickness,
//	const ImVec4& color, ImVec2 min, ImVec2 max, float width, float height) {
//
//	ImVec2 c = b::map(center, min, max, { 0, 0 }, { width, height });
//	float r	    = b::map(radius, 0, max.x - min.x, 0, width);
//	float t     = b::map(thickness, 0, max.x - min.x, 0, width);
//
//	//Battery::Renderer2D::DrawArc(c, r, startAngle, endAngle, t, color, EXPORT_FALLOFF);
//}
//
//void ApplicationRenderer::DrawRectangleWorkspace(const ImVec2& point1, const ImVec2& point2, float outlineThickness,
//	const ImColor& outlineColor, const ImColor& fillColor) {
//	ImVec2 p1 = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(point1);
//	ImVec2 p2 = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(point2);
//	//Battery::Renderer2D::DrawRectangle(p1, p2, outlineThickness, outlineColor, fillColor, falloff);
//}
//
//void ApplicationRenderer::DrawRectangleScreenspace(const ImVec2& point1, const ImVec2& point2, float outlineThickness,
//	const ImColor& outlineColor, const ImColor& fillColor) {
//	//Battery::Renderer2D::DrawRectangle(point1, point2, outlineThickness, outlineColor, fillColor, falloff);
//}
//
//
//
//
//
//void ApplicationRenderer::DrawSelectionBoxInfillRectangle(const ImVec2& point1, const ImVec2& point2) {
//	ApplicationRenderer::DrawRectangleWorkspace(point1, point2, 0, { 0, 0, 0, 0 }, GetInstance().selectionBoxFillColor);
//}
//
//void ApplicationRenderer::DrawSelectionBoxOutlineRectangle(const ImVec2& point1, const ImVec2& point2) {
//	ApplicationRenderer::DrawRectangleWorkspace(point1, point2,
//		GetInstance().selectionBoxOutlineThickness, GetInstance().selectionBoxOutlineColor, { 0, 0, 0, 0 });
//}
//
//void ApplicationRenderer::DrawPreviewPoint(const ImVec2& position) {
//	ImVec2 p1 = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(position);
//	ApplicationRenderer::DrawRectangleScreenspace(
//		p1 - ImVec2(GetInstance().previewPointSize / 2, GetInstance().previewPointSize / 2),
//		p1 + ImVec2(GetInstance().previewPointSize / 2, GetInstance().previewPointSize / 2),
//		1, { 0, 0, 0, 255 }, { 255, 255, 255, 255 });
//}
//
//void ApplicationRenderer::DrawGrid(bool infinite) {
//	auto nav = Navigator::GetInstance();
//
//    auto thickness = GetInstance().gridLineWidth;
//    auto alpha = std::min(Navigator::GetInstance()->scale * GetInstance().gridAlphaFactor + GetInstance().gridAlphaOffset, GetInstance().gridAlphaMax);
//	ImVec4 color = ImVec4(GetInstance().gridLineColor, GetInstance().gridLineColor, GetInstance().gridLineColor, alpha);
//
//	auto [ w, h ] = App::s_mainWindow->getSize();
//
//	float right = static_cast<float>(w);
//    float left = 0;
//    float top = static_cast<float>(h);
//    float bottom = 0;
//
//	if (!infinite) {	// Draw an A4-sheet
//		ImVec2 sheetSize = { 210, 297 };
//		right  = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords( sheetSize / 2.f).x;
//		left   = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(-sheetSize / 2.f).x;
//		top    = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords( sheetSize / 2.f).y;
//		bottom = Navigator::GetInstance()->ConvertWorkspaceToScreenCoords(-sheetSize / 2.f).y;
//
//		for (float x = nav->m_panOffset.x + w / 2; x < right; x += nav->scale * nav->snapSize) {
//			//Renderer2D::DrawPrimitiveLine({ x, bottom }, { x, top }, thickness, color);
//		}
//		for (float x = nav->m_panOffset.x + w / 2 - nav->scale * nav->snapSize; x > left; x -= nav->scale * nav->snapSize) {
//			//Renderer2D::DrawPrimitiveLine({ x, bottom }, { x, top }, thickness, color);
//		}
//		for (float y = nav->m_panOffset.y + h / 2; y < top; y += nav->scale * nav->snapSize) {
//			//Renderer2D::DrawPrimitiveLine({ left, y }, { right, y }, thickness, color);
//		}
//		for (float y = nav->m_panOffset.y + h / 2 - nav->scale * nav->snapSize; y > bottom; y -= nav->scale * nav->snapSize) {
//			//Renderer2D::DrawPrimitiveLine({ left, y }, { right, y }, thickness, color);
//		}
//	}
//	else {
//
//		for (float x = nav->m_panOffset.x + w / 2; x < right; x += nav->scale * nav->snapSize) {
//			//Renderer2D::DrawPrimitiveLine({ x, bottom }, { x, top }, thickness, color);
//		}
//		for (float x = nav->m_panOffset.x + w / 2; x > left; x -= nav->scale * nav->snapSize) {
//			//Renderer2D::DrawPrimitiveLine({ x, bottom }, { x, top }, thickness, color);
//		}
//		for (float y = nav->m_panOffset.y + h / 2; y < top; y += nav->scale * nav->snapSize) {
//			//Renderer2D::DrawPrimitiveLine({ left, y }, { right, y }, thickness, color);
//		}
//		for (float y = nav->m_panOffset.y + h / 2; y > bottom; y -= nav->scale * nav->snapSize) {
//			//Renderer2D::DrawPrimitiveLine({ left, y }, { right, y }, thickness, color);
//		}
//	}
//
//	if (!infinite) {	// Draw sheet outline
//		//Renderer2D::DrawPrimitiveLine({ left,  bottom }, { right, bottom }, thickness * 2, color);
//		//Renderer2D::DrawPrimitiveLine({ right, bottom }, { right, top }, thickness * 2, color);
//		//Renderer2D::DrawPrimitiveLine({ right, top},     { left,  top }, thickness * 2, color);
//		//Renderer2D::DrawPrimitiveLine({ left,  top },    { left, bottom }, thickness * 2, color);
//	}
//}
