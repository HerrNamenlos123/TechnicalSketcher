
#include "pch.h"
#include "Application.h"
#include "utils.h"


bool Application::saveFile(const std::string& file) {

	// Save all layers and shapes in this json object
	nlohmann::json json;

	try {
		json = layers.getJson();
	}
	catch (...) {
		std::cout << "Error: Failed to construct json object" <<
			" in Application::saveFile(), no further information" << std::endl;
		return false;
	}

	//std::cout << json.dump(4) << std::endl;

	// Save to file
	bool success = saveStringToFile(file, json.dump(4));

	if (success) {
		std::cout << "Saved file '" << file << "'" << std::endl;
	}
	else {
		std::cout << "Failed to save file '" << file << "'" << std::endl;
	}

	return success;
}
