
#include "pch.h"
#include "Application.h"
#include "utils.h"
#include <windows.h>


std::string getSaveFileName(Application* app) {

    char fname[MAX_PATH];

    OPENFILENAMEA ofn;
    ZeroMemory(&fname, sizeof(fname));
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = al_get_win_window_handle(app->display);
    ofn.lpstrFilter = "TechnicalSketcher Project (*.tsk)\0*.tsk\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = fname;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = "Save as";
    ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

    if (GetSaveFileNameA(&ofn)) {
        return std::string(fname);
    }
    else {
        // All this stuff below is to tell you exactly how you messed up above. 
        // Once you've got that fixed, you can often (not always!) reduce it to a 'user cancelled' assumption.
        switch (CommDlgExtendedError())
        {
        case CDERR_DIALOGFAILURE: app->errorMessage("CDERR_DIALOGFAILURE"); break;
        case CDERR_FINDRESFAILURE: app->errorMessage("CDERR_FINDRESFAILURE"); break;
        case CDERR_INITIALIZATION: app->errorMessage("CDERR_INITIALIZATION"); break;
        case CDERR_LOADRESFAILURE: app->errorMessage("CDERR_LOADRESFAILURE"); break;
        case CDERR_LOADSTRFAILURE: app->errorMessage("CDERR_LOADSTRFAILURE"); break;
        case CDERR_LOCKRESFAILURE: app->errorMessage("CDERR_LOCKRESFAILURE"); break;
        case CDERR_MEMALLOCFAILURE: app->errorMessage("CDERR_MEMALLOCFAILURE"); break;
        case CDERR_MEMLOCKFAILURE: app->errorMessage("CDERR_MEMLOCKFAILURE"); break;
        case CDERR_NOHINSTANCE: app->errorMessage("CDERR_NOHINSTANCE"); break;
        case CDERR_NOHOOK: app->errorMessage("CDERR_NOHOOK"); break;
        case CDERR_NOTEMPLATE: app->errorMessage("CDERR_NOTEMPLATE"); break;
        case CDERR_STRUCTSIZE: app->errorMessage("CDERR_STRUCTSIZE"); break;
        case FNERR_BUFFERTOOSMALL: app->errorMessage("FNERR_BUFFERTOOSMALL"); break;
        case FNERR_INVALIDFILENAME: app->errorMessage("FNERR_INVALIDFILENAME"); break;
        case FNERR_SUBCLASSFAILURE: app->errorMessage("FNERR_SUBCLASSFAILURE"); break;
        default: break; // File Cancelled
        }
    }

    return "";
}


bool Application::saveFile(bool saveAs) {

	// Save all layers and shapes in this json object
	nlohmann::json json;

	try {
		json = file.getJson();
	}
	catch (...) {
        errorMessage("Error: Failed to construct json object in Application::saveFile()");
		return false;
	}

	//std::cout << json.dump(4) << std::endl;


    // Get the file location
    if (file.__fileLocation == "" || saveAs) {
        // No location is known yet
        file.__fileLocation = getSaveFileName(this);

        if (file.__fileLocation == "") {   // Invalid
            return false;
        }
    }

    // fileLocation must be valid now

    std::cout << "Saving file '" << file.__fileLocation << "'" << std::endl;

    // Save to file
    if (!saveStringToFile(file.__fileLocation, json.dump(4))) {
        errorMessage("Failed to save file '" + file.__fileLocation + "'");
        return false;
    }

    file.__fileChanged = false;

    // Set window title
    al_set_window_title(display, (file.filename + " - TechnicalSketcher").c_str());

	return true;
}
