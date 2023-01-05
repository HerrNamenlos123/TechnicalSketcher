
#include "pch.h"
#include "SketchLayer.h"

#include "Shapes/GenericShape.h"

SketchLayer::SketchLayer(const std::string& name) {
	id = nextID++;
	this->name = name;
}

LayerState SketchLayer::GetState() {
	return state;
}

void SketchLayer::LoadState(const LayerState& state) {
	this->state = state;
}

SketchLayer SketchLayer::Duplicate() {
	SketchLayer layer(name + " - copy");

	layer.LoadState(state);
	layer.layerChanged = true;
	layer.history = history;

	return layer;
}

LayerID SketchLayer::GetID() const {
	return id;
}

void SketchLayer::SetID(LayerID id) {
	this->id = id;
}

const std::vector<ShapePTR>& SketchLayer::GetShapes() const {
	return state.GetShapes();
}

bool SketchLayer::AddShape(const nlohmann::json& json) {
	SaveState();
	ShapePTR shape = GenericShape::MakeShape(json);
	if (shape) {
		LOG_TRACE("Shape added to layer {} with id ", shape->GetID());
		state.PushShape(std::move(shape));
		return true;
	}
	return false;
}

void SketchLayer::AddShape(enum class ShapeType type, glm::vec2 p1, glm::vec2 p2, float thickness, const glm::vec4& color) {
	SaveState();
	ShapePTR shape = GenericShape::MakeShape(type, p1, p2, thickness, color);
	if (shape) {
		LOG_TRACE("Shape added to layer {} with id ", shape->GetID());
		state.PushShape(std::move(shape));
	}
}

void SketchLayer::AddShape(enum class ShapeType type, glm::vec2 center, float radius, float thickness, const glm::vec4& color) {
	SaveState();
	ShapePTR shape = GenericShape::MakeShape(type, center, radius, thickness, color);
	if (shape) {
		LOG_TRACE("Shape added to layer {} with id ", shape->GetID());
		state.PushShape(std::move(shape));
	}
}

void SketchLayer::AddShape(enum class ShapeType type, glm::vec2 center, float radius, float startAngle, float endAngle, float thickness, const glm::vec4& color) {
	SaveState();
	ShapePTR shape = GenericShape::MakeShape(type, center, radius, startAngle, endAngle, thickness, color);
	if (shape) {
		LOG_TRACE("Shape added to layer {} with id ", shape->GetID());
		state.PushShape(std::move(shape));
	}
}

bool SketchLayer::AddShapes(const std::vector<nlohmann::json>& jsonArray) {

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

void SketchLayer::AddShapes(std::vector<ShapePTR>&& shapes) {
	// Apply the shapes
	SaveState();
	for (auto& shape : shapes) {
		state.PushShape(std::move(shape));
	}
}

bool SketchLayer::RemoveShape(const ShapeID& id) {
	SaveState();
	LOG_TRACE("Removing shape #{}", id);
	return state.RemoveShape(id);
}

bool SketchLayer::RemoveShapes(const std::vector<ShapeID>& ids) {
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

bool SketchLayer::MoveShapeLeft(const ShapeID& id, float amount) {
	SaveState();
	auto shape = FindShape(id);

	if (shape.has_value()) {
		shape.value().get().MoveLeft(amount);
		return true;
	}

	return false;
}

bool SketchLayer::MoveShapeRight(const ShapeID& id, float amount) {
	SaveState();
	auto shape = FindShape(id);

	if (shape) {
		shape.value().get().MoveRight(amount);
		return true;
	}

	return false;
}

bool SketchLayer::MoveShapeUp(const ShapeID& id, float amount) {
	SaveState();
	auto shape = FindShape(id);

	if (shape) {
		shape.value().get().MoveUp(amount);
		return true;
	}

	return false;
}

bool SketchLayer::MoveShapeDown(const ShapeID& id, float amount) {
	SaveState();
	auto shape = FindShape(id);

	if (shape) {
		shape.value().get().MoveDown(amount);
		return true;
	}

	return false;
}

bool SketchLayer::MoveShapesLeft(const std::vector<ShapeID>& ids, float amount) {
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

bool SketchLayer::MoveShapesRight(const std::vector<ShapeID>& ids, float amount) {
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

bool SketchLayer::MoveShapesUp(const std::vector<ShapeID>& ids, float amount) {
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

bool SketchLayer::MoveShapesDown(const std::vector<ShapeID>& ids, float amount) {
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

bool SketchLayer::MoveShapes(const std::vector<ShapeID>& ids, glm::vec2 amount) {
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

void SketchLayer::SaveState() {
	// Save state for undo
	history.PushState(GetState());
}

void SketchLayer::UndoAction() {

	auto pair = history.PopState();

	if (pair.second) {
		LoadState(pair.first);
	}
	else {
		LOG_WARN("Can't undo action: No more actions are stored!");
	}
}

/*void SketchLayer::SetPreviewImage(const sf::Image& image) {
	previewImage = image;
}

void SketchLayer::GeneratePreview() {
	previewImage.Reset();
	previewImage = Battery::Bitmap();
	previewImage.CreateBitmap(GUI_PREVIEWWINDOW_SIZE, GUI_PREVIEWWINDOW_SIZE);
	RenderLayerToBitmap(previewImage.GetAllegroBitmap());
}*/

std::optional<std::reference_wrapper<GenericShape>> SketchLayer::FindShape(const ShapeID& shape) {
	return state.FindShape(shape);
}

bool SketchLayer::ShapeExists(const ShapeID& id) const {
	return state.ShapeExists(id);
}

std::pair<glm::vec2, glm::vec2> SketchLayer::GetBoundingBox() const {
	return state.GetBoundingBox();
}

float SketchLayer::MapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void SketchLayer::RenderLayerToBitmap(sf::RenderTexture texture) {
	/*const auto& shapes = GetShapes();

	// Save current draw buffer to return to later
	ALLEGRO_BITMAP* previousBuffer = al_get_target_bitmap();

	ApplicationRenderer::BeginFrame();

	// If no shapes, just render white
	if (shapes.size() == 0) {
		al_set_target_bitmap(bitmap);
		Battery::Renderer2D::DrawBackground({ 255, 255, 255, 255 });
		ApplicationRenderer::EndFrame();
		al_set_target_bitmap(previousBuffer);
		return;
	}

	// Calculate the encapsulated frame
	glm::vec2 min = shapes[0]->GetBoundingBox().first;
	glm::vec2 max = shapes[0]->GetBoundingBox().second;

	for (const auto& s : shapes) {

		auto smin = s->GetBoundingBox().first;
		auto smax = s->GetBoundingBox().second;

		if (smin.x < min.x) min.x = smin.x;
		if (smin.y < min.y) min.y = smin.y;
		if (smax.x > max.x) max.x = smax.x;
		if (smax.y > max.y) max.y = smax.y;
	}

	float brim = 1.2f;
	glm::vec2 center = { (max.x + min.x) / 2.f, (max.y + min.y) / 2.f };
	int width = al_get_bitmap_width(bitmap);
	int height = al_get_bitmap_height(bitmap);
	float range = 0;
	if (width / (max.x - min.x) <= height / (max.y - min.y)) {	// X is larger, adapt Y
		range = max.x - min.x;
	}
	else {		// Y is larger, adapt X
		range = max.y - min.y;
	}
	min.x = center.x - range / 2 * brim;
	max.x = center.x + range / 2 * brim;
	min.y = center.y - range / 2 * brim;
	max.y = center.y + range / 2 * brim;

	al_set_target_bitmap(bitmap);
	Battery::Renderer2D::DrawBackground({ 255, 255, 255, 255 });

	for (const auto& s : shapes) {
		s->RenderExport(min, max, width, height);
	}

	ApplicationRenderer::EndFrame();
	al_set_target_bitmap(previousBuffer);*/
}

bool SketchLayer::LoadJson(nlohmann::json json) {

	/*try {
		std::string name = json["name"];
		LayerState state;
		if (state.LoadJson(json["shapes"])) {
			this->name = name;
			this->state = state;
			history.Clear();
			previewImage = Battery::Bitmap();
			layerChanged = false;
			return true;
		}
	}
	catch (...) {
		LOG_ERROR("Can't parse json: Invalid format!");
	}*/

	return false;
}

nlohmann::json SketchLayer::GetJson() {
	nlohmann::json j = nlohmann::json();

	j["shapes"] = state.GetJson();
	j["name"] = name;

	return j;
}
