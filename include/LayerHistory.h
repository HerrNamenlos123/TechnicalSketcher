//#pragma once
//
//#include "pch.h"
//#include "LayerState.h"
//
//template <size_t capacity>
//class LayerHistory {
//
//	std::vector<LayerState> states;
//
//public:
//	LayerHistory() {}
//
//	void PushState(const LayerState& state) {
//		states.push_back(state);
//
//		// Delete the first elements if there's too many
//		while (states.size() > capacity) {
//			states.erase(states.begin());
//		}
//	}
//
//	std::pair<LayerState, bool> PopState() {
//
//		if (states.size() == 0) {
//			return std::make_pair<LayerState, bool>(LayerState(), false);
//		}
//
//		std::pair<LayerState, bool> pair = std::make_pair<LayerState, bool>(LayerState(states[states.size() - 1]), true);
//		states.pop_back();
//		return pair;
//	}
//
//	void Clear() {
//		states.clear();
//	}
//};
