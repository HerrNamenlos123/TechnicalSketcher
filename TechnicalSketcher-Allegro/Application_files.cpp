
#include "pch.h"
#include "Application.h"
#include "utils.h"


bool Application::saveFile(const std::string& filename) {

	// Save all layers and shapes in this json object
	nlohmann::json json;

	try {
		json = file.getJson();
	}
	catch (...) {
		std::cout << "Error: Failed to construct json object" <<
			" in Application::saveFile(), no further information" << std::endl;
		return false;
	}

	//std::cout << json.dump(4) << std::endl;

	// Save to file
	bool success = saveStringToFile(filename, json.dump(4));

	if (success) {
		std::cout << "Saved file '" << filename << "'" << std::endl;
	}
	else {
		std::cout << "Failed to save file '" << filename << "'" << std::endl;
	}

	return success;
}
