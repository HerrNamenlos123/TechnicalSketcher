#pragma once

#include "pch.h"
#include "Shape.h"

typedef size_t LayerID;

class Layer {

	std::vector<Shape> shapes;
	ShapeID maxShapeID = 0;

public:
	std::string name;
	LayerID layerID = -1;
	ALLEGRO_BITMAP* bitmap = nullptr;

	Layer(const std::string& n, LayerID id) {
		name = n;
		bitmap = nullptr;
		layerID = id;
	}

	Layer(const Layer& layer) {
		name = layer.name;
		shapes = layer.shapes;
		bitmap = nullptr;
		layerID = layer.layerID;
	};

	void operator=(const Layer& layer) {
		name = layer.name;
		shapes = layer.shapes;
		bitmap = nullptr;
		layerID = layer.layerID;
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

		ShapeID id = maxShapeID;
		maxShapeID++;

		shapes.push_back(Shape(id, type, p1, p2, thickness));
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

	std::vector<Shape>& getShapes() {
		return shapes;
	}

	size_t size() {
		return shapes.size();
	}

	Shape& findShape(ShapeID shape) {

		for (size_t i = 0; i < shapes.size(); i++) {
			if (shapes[i].shapeID == shape) {
				return shapes[i];
			}
		}

		throw std::logic_error("Can't find shape with ID " + std::to_string(shape) + " in layer '" + name + "'");
	}
};
