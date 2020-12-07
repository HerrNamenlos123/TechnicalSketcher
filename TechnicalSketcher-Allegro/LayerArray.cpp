
#include "pch.h"
#include "LayerArray.h"
#include "MainEngine.h"


LayerArray::LayerArray(MainEngine* reference) : dummyLayer("dummy", -1), engineReference(reference) {

}

void LayerArray::addLayerFront(const std::string& name) {
	layers.insert(layers.begin(), Layer(name, nextLayerID));

	if (selectedLayer == -1) {
		selectedLayer = nextLayerID;
	}

	nextLayerID++;
}

void LayerArray::addLayerBack(const std::string& name) {
	layers.push_back(Layer(name, nextLayerID));

	if (selectedLayer == -1) {
		selectedLayer = nextLayerID;
	}

	nextLayerID++;
}

bool LayerArray::moveLayerFront(size_t index) {
	if (index > 0 && index < layers.size()) {
		std::iter_swap(layers.begin() + index, layers.begin() + index - 1);
		return true;
	}
	return false;
}

bool LayerArray::moveLayerBack(size_t index) {
	if (index >= 0 && index < layers.size() - 1) {
		std::iter_swap(layers.begin() + index, layers.begin() + index + 1);
		return true;
	}
	return false;
}

Layer& LayerArray::findLayer(LayerID id) {

	for (Layer& layer : layers) {
		if (layer.layerID == id) {
			return layer;
		}
	}

	throw std::logic_error("Can't find Layer with ID " + std::to_string(id) + " in LayerArray");
}

void LayerArray::updateGUI() {

	ImGui::PushFont(engineReference->font_pt22);
	float fontHeight = 22;					// Font height
	float itemHeight = fontHeight + 4;		// Font height + 4 pixels
	ImGuiIO& io = ImGui::GetIO();

	for (size_t i = 0; i < layers.size(); i++) {
		Layer& layer = layers[i];

		std::string name = layer.name + "##" + std::to_string(layer.layerID);

		// Draw GUI element
		if (ImGui::Selectable(name.c_str(), selectedLayer == layer.layerID)) {
			selectedLayer = layer.layerID;
			engineReference->events_layerSelectedEventFlag = true;
			engineReference->gui_previewChanged = true;
		}

		// Drag and drop
		if (ImGui::IsItemActive() && !ImGui::IsItemHovered() && 
			engineReference->mouse.x <= engineReference->gui_leftUpperViewportCorner.x)
		{
			int n_next = static_cast<int>(i) + (ImGui::GetMouseDragDelta(0).y < 0.f ? -1 : 1);
			if (n_next >= 0 && n_next < layers.size())
			{
				if (n_next > i) {
					engineReference->events_moveLayerBackID = i;
				}
				else {
					engineReference->events_moveLayerFrontID = i;
				}
				ImGui::ResetMouseDragDelta();
			}
		}

		// Draw the preview of the layer
		if (ImGui::IsItemHovered())
		{
			if (layers[i].bitmap != nullptr) {
				ImGui::BeginTooltip();
				ImGui::Image(layers[i].bitmap, ImVec2(engineReference->gui_hoverWindowSize.x, engineReference->gui_hoverWindowSize.y));
				ImGui::EndTooltip();
			}
		}
	}

	ImGui::PopFont();
}
