
#include "pch.h"
#include "Application.h"
#include "utils.h"
#include <windows.h>
#include <shlobj.h>


bool inAlphabet(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

std::string getSaveFilename(Application* app) {

    // Get Documents folder
    ALLEGRO_PATH* doc = al_get_standard_path(ALLEGRO_USER_DOCUMENTS_PATH);
    std::string documents = std::string(al_path_cstr(doc, '/'));
    al_destroy_path(doc);

    // Instantiate popup options
    ALLEGRO_FILECHOOSER* file = al_create_native_file_dialog(
        documents.c_str(),
        "Save as",
        "*.*;*.tsk;",
        ALLEGRO_FILECHOOSER_SAVE
    );

    // Open queued window
    if (!al_show_native_file_dialog(app->display, file)) {
        al_destroy_native_file_dialog(file);
        return "";       // File was cancelled
    }

    // File was valid
    std::string path = al_get_native_file_dialog_path(file, 0);
    ALLEGRO_PATH* p = al_create_path(path.c_str());
    std::string extension = al_get_path_extension(p);
    al_destroy_path(p);

    std::cout << "Extension: " << extension << std::endl;

    // Check file extension
    if (extension != ".tsk") {

        if (path[path.length() - 1] != '/') {
            path += ".tsk";

            // Alert if file exists already
            if (al_filename_exists(path.c_str())) {
                ALLEGRO_PATH* p1 = al_create_path(path.c_str());
                int r = al_show_native_message_box(app->display, 
                            "Confirm Save as", 
                            (std::string(al_get_path_filename(p1)) + " already exists.").c_str(),
                            "Do you really want to overwrite it?", 
                            NULL, 
                            ALLEGRO_MESSAGEBOX_YES_NO | ALLEGRO_MESSAGEBOX_WARN
                        );
                al_destroy_path(p1);

                // If r == 1, yes was pressed, then continue and overwrite file
                if (r != 1) {
                    return "";
                }
            }
        }
        else {
            app->errorMessage("Invalid filename: " + path);
            return "";
        }
    }

    // Free memory
    al_destroy_native_file_dialog(file);

    return path;
}


void Application::updateWindowTitle() {
    al_set_window_title(display, (file.filename + " - TechnicalSketcher").c_str());
}


bool Application::saveFile(bool saveAs) {

	// Save all layers and shapes in this json object
	nlohmann::json json;

	try {
		json = file.getJson();
	}
	catch (...) {
        errorMessage("[Application::saveFile(bool)]: Failed to construct JSON object");
		return false;
	}

	//std::cout << json.dump(4) << std::endl;

    // Get the file location
    if (file.__fileLocation == "" || saveAs) {
        // No location is known yet
        file.__fileLocation = getSaveFilename(this);

        if (file.__fileLocation == "") {   // Invalid
            return false;
        }
    }

    std::cout << "Saving file '" << file.__fileLocation << "'" << std::endl;

    // Save to file
    if (!saveStringToFile(file.__fileLocation, json.dump(4))) {
        errorMessage("Failed to save file '" + file.__fileLocation + "'");
        return false;
    }

    file.__fileChanged = false;

    // Set correct display name
    ALLEGRO_PATH* path = al_create_path(file.__fileLocation.c_str());
    file.filename = al_get_path_filename(path);
    al_destroy_path(path);

    // Set window title
    updateWindowTitle();

    infoMessage("Saved file " + file.__fileLocation);

	return true;
}
