#pragma once

#include "pch.h"
#include "Shape.h"

typedef size_t LayerID;

class Layer {

	std::vector<Shape> shapes;
	ShapeID nextShapeID = 0;

public:
	std::string name;
	LayerID layerID = -1;
	ALLEGRO_BITMAP* bitmap = nullptr;

	Layer(const std::string& n, LayerID id) {
		name = n;
		bitmap = nullptr;
		layerID = id;
		nextShapeID = 0;
	}

	Layer(const Layer& layer) {
		name = layer.name;
		shapes = layer.shapes;
		bitmap = nullptr;
		layerID = layer.layerID;
		nextShapeID = layer.nextShapeID;
	};

	Layer(nlohmann::json j) {
		name = j["name"].get<std::string>();
		bitmap = nullptr;
		layerID = j["id"];
		nextShapeID = j["next_layer_id"];

		for (nlohmann::json sh : j["shapes"]) {
			shapes.push_back(Shape(sh));
		}
	};

	void operator=(const Layer& layer) {
		name = layer.name;
		shapes = layer.shapes;
		bitmap = nullptr;
		layerID = layer.layerID;
		nextShapeID = layer.nextShapeID;
	};

	~Layer() {
		al_destroy_bitmap(bitmap);
		bitmap = nullptr;
	}

	void cloneFrom(ALLEGRO_BITMAP* bmp) {
		al_destroy_bitmap(bitmap);
		bitmap = al_clone_bitmap(bmp);
	}

	void addShape(enum ShapeType type, glm::vec2 p1, glm::vec2 p2, float thickness) {

		//std::cout << "Shape added to layer " << name << " with id " << nextShapeID << std::endl;

		shapes.push_back(Shape(nextShapeID, type, p1, p2, thickness));
		nextShapeID++;
	}

	bool removeShape(ShapeID shape) {

		for (int i = 0; i < shapes.size(); i++) {
			if (shapes[i].shapeID == shape) {
				shapes.erase(shapes.begin() + i);
				return true;
			}
		}

		return false;
	}

	std::vector<Shape*> getShapes() {

		std::vector<Shape*> s;

		for (size_t i = 0; i < shapes.size(); i++) {
			s.push_back(&shapes[i]);
		}

		return s;
	}

	size_t size() {
		return shapes.size();
	}

	Shape* findShape(ShapeID shape) {

		for (size_t i = 0; i < shapes.size(); i++) {
			if (shapes[i].shapeID == shape) {
				return &shapes[i];
			}
		}

		return nullptr;
	}

	nlohmann::json getJson() {

		// Convert all data to a json object

		nlohmann::json json;

		nlohmann::json jsonShapes = nlohmann::json::array();
		for (Shape& shape : shapes) {
			jsonShapes.push_back(shape.getJson());
		}

		json = nlohmann::json{
			{ "id", layerID },
			{ "name", name },
			{ "next_layer_id", nextShapeID },
			{ "shapes", jsonShapes }
		};

		return json;
	}
};
