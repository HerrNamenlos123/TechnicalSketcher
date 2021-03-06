#pragma once
/*
#include "pch.h"
#include "Shape.h"

// LayerID gets its own class to make using it wrongly impossible
// -1 is invalid
// The ID gets assigned automatically and is always unique
class LayerID {

	int32_t id = -1;
	static int32_t nextID;

public:
	LayerID() {
		id = -1;
	}

	LayerID(const LayerID& id) {
		this->id = id.id;
	}

	LayerID(int32_t id) {
		this->id = id;
	}

	static int32_t MakeID() {
		LayerID newID(nextID);
		nextID++;
		return newID;
	}

	int32_t Get() const {
		return id;
	}

	bool IsValid() const {
		return id == -1;
	}

	operator int32_t() const {
		return id;
	}

	bool operator==(const LayerID& id) {
		return this->id == id.id;
	}
};

class Layer {

	std::vector<Shape> shapes;

public:
	std::string name;
	LayerID layerID;
	Battery::Texture2D previewImage;

	Layer(const std::string& n) {
		name = n;
		layerID = LayerID::MakeID();
	}

	Layer(const Layer& layer) {
		name = layer.name;
		shapes = layer.shapes;
		layerID = layer.layerID;
		previewImage = layer.previewImage;
	};

	void operator=(const Layer& layer) {
		name = layer.name;
		shapes = layer.shapes;
		layerID = layer.layerID;
		previewImage = previewImage;
	};

	void operator=(const Layer&& layer) {
		name = layer.name;
		shapes = std::move(layer.shapes);
		layerID = layer.layerID;
		previewImage = std::move(previewImage);
	};

	//void cloneFrom(ALLEGRO_BITMAP* bmp) {
	//	al_destroy_bitmap(bitmap);
	//	bitmap = al_clone_bitmap(bmp);
	//}

	void AddShape(enum class ShapeType type, glm::vec2 p1, glm::vec2 p2, float thickness) {
		Shape shape(type, p1, p2, thickness);
		shapes.push_back(std::move(shape));
		LOG_TRACE("Shape added to layer " + name + " with id " + std::to_string(shape.shapeID.Get()));
	}

	bool RemoveShape(const ShapeID& shape) {

		for (size_t i = 0; i < shapes.size(); i++) {
			if (shapes[i].shapeID == shape) {
				shapes.erase(shapes.begin() + i);
				return true;
			}
		}

		return false;
	}

	std::vector<Shape*> GetShapes() {

		std::vector<Shape*> s;

		for (size_t i = 0; i < shapes.size(); i++) {
			s.push_back(&shapes[i]);
		}

		return s;
	}

	size_t Size() {
		return shapes.size();
	}

	Shape* FindShape(const ShapeID& shape) {

		for (size_t i = 0; i < shapes.size(); i++) {
			if (shapes[i].shapeID == shape) {
				return &shapes[i];
			}
		}

		return nullptr;
	}
};
*/

#include "pch.h"
#include "Shape.h"

// LayerID gets its own class to make using it wrongly impossible
// -1 is invalid
// The ID gets assigned automatically and is always unique
class LayerID {

	int32_t id = -1;
	static int32_t nextID;

public:
	LayerID() {
		id = -1;
	}

	LayerID(const LayerID& id) {
		this->id = id.id;
	}

	LayerID(int32_t id) {
		this->id = id;
	}

	static int32_t MakeID() {
		LayerID newID(nextID);
		nextID++;
		return newID;
	}

	int32_t Get() const {
		return id;
	}

	bool IsValid() const {
		return id == -1;
	}

	operator int32_t() const {
		return id;
	}

	bool operator==(const LayerID& id) {
		return this->id == id.id;
	}
};

class Layer {

	std::vector<Shape> shapes;

public:
	std::string name;
	LayerID layerID;
	Battery::Texture2D previewImage;

	Layer(const std::string& name) {
		this->name = name;
		layerID = LayerID::MakeID();
	}
	
	void AddShape(enum class ShapeType type, glm::vec2 p1, glm::vec2 p2, float thickness) {
		Shape shape(type, p1, p2, thickness);
		LOG_TRACE("Shape added to layer " + name + " with id " + std::to_string(shape.shapeID.Get()));
		shapes.push_back(std::move(shape));
	}
	
	bool RemoveShape(const ShapeID& id) {

		for (size_t i = 0; i < shapes.size(); i++) {
			if (shapes[i].shapeID == id) {
				shapes.erase(shapes.begin() + i);
				return true;
			}
		}

		return false;
	}

	void SetPreviewImage(const Battery::Texture2D& image) {
		previewImage = image;
	}
	
	std::vector<Shape*> GetShapes() {

		std::vector<Shape*> s;

		for (size_t i = 0; i < shapes.size(); i++) {
			s.push_back(&shapes[i]);
		}

		return s;
	}

	size_t Size() {
		return shapes.size();
	}

	Shape* FindShape(const ShapeID& shape) {

		for (size_t i = 0; i < shapes.size(); i++) {
			if (shapes[i].shapeID == shape) {
				return &shapes[i];
			}
		}

		return nullptr;
	}

	nlohmann::json GetJson() {
		return nlohmann::json();
	}
};