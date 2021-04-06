
#include "pch.h"
#include "Layer.h"

#include "Shapes/GenericShape.h"
#include "Shapes/LineShape.h"
#include "Application.h"

Layer::Layer(const std::string& name) {
	id = nextID++;
	this->name = name;
}

LayerState Layer::GetState() {
	return state;
}

void Layer::LoadState(const LayerState& state) {
	this->state = state;
}

Layer Layer::Duplicate() {
	Layer layer(name);

	layer.LoadState(state);
	layer.layerChanged = true;
	layer.history = history;

	return layer;
}

LayerID Layer::GetID() const {
	return id;
}

void Layer::SetID(LayerID id) {
	this->id = id;
}

const std::vector<ShapePTR>& Layer::GetShapes() const {
	return state.GetShapes();
}

bool Layer::AddShape(const nlohmann::json& json) {
	SaveState();
	ShapePTR shape = GenericShape::MakeShape(json);
	if (shape) {
		LOG_TRACE("Shape added to layer {} with id ", shape->GetID());
		state.PushShape(std::move(shape));
		return true;
	}
	return false;
}

void Layer::AddShape(enum class ShapeType type, glm::vec2 p1, glm::vec2 p2, float thickness, const glm::vec4& color) {
	SaveState();
	ShapePTR shape = GenericShape::MakeShape(type, p1, p2, thickness, color);
	if (shape) {
		LOG_TRACE("Shape added to layer {} with id ", shape->GetID());
		state.PushShape(std::move(shape));
	}
}

bool Layer::AddShapes(const std::vector<nlohmann::json>& jsonArray) {

	// Store shapes and only push if all are valid
	std::vector<ShapePTR> shapes;
	for (const auto& json : jsonArray) {
		shapes.push_back(GenericShape::MakeShape(json));

		if (!shapes[shapes.size() - 1]) {
			LOG_TRACE(__FUNCTION__"(): Failed to parse Shape");
			return false;
		}
	}

	// Now apply the shapes
	SaveState();
	for (auto& shape : shapes) {
		state.PushShape(std::move(shape));
	}
	return true;
}

void Layer::AddShapes(std::vector<ShapePTR>&& shapes) {
	// Apply the shapes
	SaveState();
	for (auto& shape : shapes) {
		state.PushShape(std::move(shape));
	}
}

bool Layer::RemoveShape(const ShapeID& id) {
	SaveState();
	LOG_TRACE("Removing shape #{}", id);
	return state.RemoveShape(id);
}

bool Layer::RemoveShapes(const std::vector<ShapeID>& ids) {
	bool failed = false;
	SaveState();

	for (auto shape : ids) {
		LOG_TRACE("Removing shape #{}", shape);

		if (!state.RemoveShape(shape)) {
			failed = true;
		}
	}

	return !failed;
}

bool Layer::MoveShapeLeft(const ShapeID& id, float amount) {
	SaveState();
	auto shape = FindShape(id);

	if (shape.has_value()) {
		shape.value().get().MoveLeft(amount);
		return true;
	}

	return false;
}

bool Layer::MoveShapeRight(const ShapeID& id, float amount) {
	SaveState();
	auto shape = FindShape(id);

	if (shape) {
		shape.value().get().MoveRight(amount);
		return true;
	}

	return false;
}

bool Layer::MoveShapeUp(const ShapeID& id, float amount) {
	SaveState();
	auto shape = FindShape(id);

	if (shape) {
		shape.value().get().MoveUp(amount);
		return true;
	}

	return false;
}

bool Layer::MoveShapeDown(const ShapeID& id, float amount) {
	SaveState();
	auto shape = FindShape(id);

	if (shape) {
		shape.value().get().MoveDown(amount);
		return true;
	}

	return false;
}

bool Layer::MoveShapesLeft(const std::vector<ShapeID>& ids, float amount) {
	SaveState();

	bool failed = false;
	for (auto id : ids) {
		auto shape = FindShape(id);

		if (shape) {
			shape.value().get().MoveLeft(amount);
		}
		else {
			failed = true;
		}
	}

	return !failed;
}

bool Layer::MoveShapesRight(const std::vector<ShapeID>& ids, float amount) {
	SaveState();

	bool failed = false;
	for (auto id : ids) {
		auto shape = FindShape(id);

		if (shape) {
			shape.value().get().MoveRight(amount);
		}
		else {
			failed = true;
		}
	}

	return !failed;
}

bool Layer::MoveShapesUp(const std::vector<ShapeID>& ids, float amount) {
	SaveState();

	bool failed = false;
	for (auto id : ids) {
		auto shape = FindShape(id);

		if (shape) {
			shape.value().get().MoveUp(amount);
		}
		else {
			failed = true;
		}
	}

	return !failed;
}

bool Layer::MoveShapesDown(const std::vector<ShapeID>& ids, float amount) {
	SaveState();

	bool failed = false;
	for (auto id : ids) {
		auto shape = FindShape(id);

		if (shape) {
			shape.value().get().MoveDown(amount);
		}
		else {
			failed = true;
		}
	}
	
	return !failed;
}

bool Layer::MoveShapes(const std::vector<ShapeID>& ids, glm::vec2 amount) {
	SaveState();

	bool failed = false;
	for (auto id : ids) {
		auto shape = FindShape(id);

		if (shape) {
			shape.value().get().Move(amount);
		}
		else {
			failed = true;
		}
	}

	return !failed;
}

void Layer::SaveState() {
	// Save state for undo
	history.PushState(GetState());
}

void Layer::UndoAction() {

	auto pair = history.PopState();

	if (pair.second) {
		LoadState(pair.first);
	}
	else {
		LOG_WARN("Can't undo action: No more actions are stored!");
	}
}

void Layer::SetPreviewImage(const Battery::Texture2D& image) {
	previewImage = image;
}

void Layer::GeneratePreview() {
	previewImage.Unload();
	previewImage = Battery::Texture2D();
	previewImage.CreateBitmap(GUI_PREVIEWWINDOW_SIZE, GUI_PREVIEWWINDOW_SIZE);
	RenderLayerToBitmap(previewImage.GetAllegroBitmap());
}

std::optional<std::reference_wrapper<GenericShape>> Layer::FindShape(const ShapeID& shape) {
	return state.FindShape(shape);
}

bool Layer::ShapeExists(const ShapeID& id) const {
	return state.ShapeExists(id);
}

float Layer::MapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void Layer::RenderLayerToBitmap(ALLEGRO_BITMAP* bitmap) {
	const auto& shapes = GetShapes();

	// Save current draw buffer to return to later
	ALLEGRO_BITMAP* previousBuffer = al_get_target_bitmap();

	ApplicationRenderer::BeginFrame(false);

	int sizeX = al_get_bitmap_width(bitmap);
	int sizeY = al_get_bitmap_height(bitmap);

	// If no shapes, just render white
	if (shapes.size() == 0) {
		al_set_target_bitmap(bitmap);
		al_clear_to_color(Battery::Graphics::ConvertAllegroColor({ 255, 255, 255, 255 }));
		ApplicationRenderer::EndFrame();
		al_set_target_bitmap(previousBuffer);
		return;
	}

	// Calculate the encapsulated frame
	float leftMost = shapes[0]->GetPoint1().x;
	float rightMost = shapes[0]->GetPoint1().x;
	float bottomMost = shapes[0]->GetPoint1().y;
	float topMost = shapes[0]->GetPoint1().y;

	for (const auto& s : shapes) {

		if (s->GetPoint1().x < leftMost)
			leftMost = s->GetPoint1().x;
		if (s->GetPoint1().x > rightMost)
			rightMost = s->GetPoint1().x;
		if (s->GetPoint1().y < bottomMost)
			bottomMost = s->GetPoint1().y;
		if (s->GetPoint1().y > topMost)
			topMost = s->GetPoint1().y;

		if (s->GetPoint2().x < leftMost)
			leftMost = s->GetPoint2().x;
		if (s->GetPoint2().x > rightMost)
			rightMost = s->GetPoint2().x;
		if (s->GetPoint2().y < bottomMost)
			bottomMost = s->GetPoint2().y;
		if (s->GetPoint2().y > topMost)
			topMost = s->GetPoint2().y;
	}

	glm::vec2 sourceFrameSize = { rightMost - leftMost, topMost - bottomMost };
	glm::vec2 renderFrameSize = { sizeX, sizeY };

	// Find separate scaling factors
	float scaleX = NAN;
	if (sourceFrameSize.x != 0)
		scaleX = sizeX / sourceFrameSize.x;
	float scaleY = NAN;
	if (sourceFrameSize.y != 0)
		scaleY = sizeY / sourceFrameSize.y;

	float mappedLeft = 0;
	float mappedRight = 0;
	float mappedTop = 0;
	float mappedBottom = 0;
	// X size is larger
	if (scaleX <= scaleY || (!isnan(scaleX) && isnan(scaleY))) {
		mappedLeft = leftMost;
		mappedRight = rightMost;
		mappedTop = (topMost + bottomMost) / 2.f + (leftMost - rightMost) * (static_cast<float>(sizeY) / sizeX) / 2.f;
		mappedBottom = (topMost + bottomMost) / 2.f - (leftMost - rightMost) * (static_cast<float>(sizeY) / sizeX) / 2.f;
	}
	else if (scaleX > scaleY || (isnan(scaleX) && !isnan(scaleY))) {	// Y size is larger
		mappedTop = bottomMost;
		mappedBottom = topMost;
		mappedLeft = (rightMost + leftMost) / 2.f + (bottomMost - topMost) / (static_cast<float>(sizeY) / sizeX) / 2.f;
		mappedRight = (rightMost + leftMost) / 2.f - (bottomMost - topMost) / (static_cast<float>(sizeY) / sizeX) / 2.f;
	}
	else {	// Both are NAN
		al_set_target_bitmap(bitmap);
		al_clear_to_color(Battery::Graphics::ConvertAllegroColor({ 255, 255, 255, 255 }));
		ApplicationRenderer::EndFrame();
		al_set_target_bitmap(previousBuffer);
		return;
	}

	float mappedWidth = mappedRight - mappedLeft;
	float mappedHeight = mappedTop - mappedBottom;
	float mappedCenterX = (mappedRight + mappedLeft) / 2.f;
	float mappedCenterY = (mappedTop + mappedBottom) / 2.f;

	float brim = 1.3f;
	mappedLeft = mappedCenterX - mappedWidth / 2.f * brim;
	mappedRight = mappedCenterX + mappedWidth / 2.f * brim;
	mappedBottom = mappedCenterY - mappedHeight / 2.f * brim;
	mappedTop = mappedCenterY + mappedHeight / 2.f * brim;

	al_set_target_bitmap(bitmap);
	al_clear_to_color(Battery::Graphics::ConvertAllegroColor({ 255, 255, 255, 255 }));

	for (const auto& s : shapes) {

		glm::vec2 scaled1;
		scaled1.x = MapFloat(s->GetPoint1().x, mappedLeft, mappedRight, 0, sizeX);
		scaled1.y = MapFloat(s->GetPoint1().y, mappedTop, mappedBottom, 0, sizeY);
		glm::vec2 scaled2;
		scaled2.x = MapFloat(s->GetPoint2().x, mappedLeft, mappedRight, 0, sizeX);
		scaled2.y = MapFloat(s->GetPoint2().y, mappedTop, mappedBottom, 0, sizeY);

		float thickness = MapFloat(s->GetThickness(), 0, mappedWidth, 0, sizeX);
		ApplicationRenderer::DrawLineScreenspace(scaled1, scaled2, thickness, s->GetColor());
	}

	ApplicationRenderer::EndFrame();
	al_set_target_bitmap(previousBuffer);
}

bool Layer::LoadJson(nlohmann::json json) {

	try {
		std::string name = json["name"];
		LayerState state;
		if (state.LoadJson(json["shapes"])) {
			this->name = name;
			this->state = state;
			history.Clear();
			previewImage = Battery::Texture2D();
			layerChanged = false;
			return true;
		}
	}
	catch (...) {
		LOG_ERROR("Can't parse json: Invalid format!");
	}

	return false;
}

nlohmann::json Layer::GetJson() {
	nlohmann::json j = nlohmann::json();

	j["shapes"] = state.GetJson();
	j["name"] = name;

	return j;
}
