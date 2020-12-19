
#include "pch.h"
#include "Application.h"
#include "utils.h"
#include <windows.h>
#include <shlobj.h>


bool inAlphabet(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}


void Application::updateWindowTitle() {
    if (file.__fileChanged)
        al_set_window_title(display, ("*" + file.filename + " - TechnicalSketcher").c_str());
    else
        al_set_window_title(display, (file.filename + " - TechnicalSketcher").c_str());
}


bool Application::closeFile() {
    
    if (file.__fileChanged) {

        // Ask the user if file needs to be saved
        int r = al_show_native_message_box(display,
            "Save changes",
            ("'" + file.filename + "' contains unsaved changes").c_str(),
            "Do you want to save it?",
            NULL,
            ALLEGRO_MESSAGEBOX_YES_NO | ALLEGRO_MESSAGEBOX_WARN
        );

        if (r == 1) {	// Yes clicked  
            if (!saveFile()) {   // Save file
                return false;       // Say that closing was not successful
            }
        }
    }

    // Now just delete everything
    file.openNewFile();

    return true;
}



std::string getOpenFilename(Application* app) {

    // Get Documents folder
    ALLEGRO_PATH* doc = al_get_standard_path(ALLEGRO_USER_DOCUMENTS_PATH);
    std::string documents = std::string(al_path_cstr(doc, '/'));
    al_destroy_path(doc);

    // Instantiate popup options
    ALLEGRO_FILECHOOSER* file = al_create_native_file_dialog(
        documents.c_str(),
        "Open",
        "*.*;*.tsk;",
        0
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

    // Check file extension
    if (extension != ".tsk") {

        // File extension is invalid, ask again
        app->errorMessage("Can't open '" + path + "', invalid file format");
        return getOpenFilename(app);

    }

    // Free memory
    al_destroy_native_file_dialog(file);

    return path;
}


bool Application::openFile() {

    // Get the file location
    std::string tempLocation = getOpenFilename(this);

    if (tempLocation == "") {   // Invalid, give up
        return false;
    }

    std::cout << "Opening file '" << tempLocation << "'" << std::endl;
    std::string fileContent = "";

    // Load file
    try {
        fileContent = loadStringFromFile(tempLocation);

        if (fileContent.length() == 0) {
            errorMessage("Failed to load " + tempLocation + ": File content is invalid");
            return false;
        }
    }
    catch (...) {
        errorMessage("Failed to open file '" + tempLocation + "'");
        return false;
    }

    // Close an open file
    while (!closeFile());      // Repeat until file was saved or discarded

    // Set correct display name
    ALLEGRO_PATH* path = al_create_path(tempLocation.c_str());
    std::string displayName = al_get_path_filename(path);
    al_destroy_path(path);

    // Actually load the file content to memory
    if (!file.loadFile(fileContent, tempLocation, displayName)) {
        errorMessage("Failed to load " + tempLocation + ": Could not parse file content");
        updateWindowTitle();
        return false;
    }

    // Set window title
    updateWindowTitle();

	return true;
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
                    "Do you really want to replace it?",
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

    return true;
}
