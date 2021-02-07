/*
#include "Application.h"

#include <fstream>
#include <filesystem>

Application::Application() : perlin(12345) {

}

bool Application::Setup(std::vector<std::string> args) {
	using namespace Battery::FileUtils;
	using namespace Battery::StringUtils;

	// Check for development parameter
	devLaunch = false;
	if (args.size() >= 2) {
		if (args[1] == "dev") {
			devLaunch = true;
		}
	}

	// Prevent ImGui from saving a .ini file
	ImGui::GetIO().IniFilename = NULL;

	// Setup various things
	updaterThread = new std::thread(updateApplication, devLaunch);	// Updater will run in the background
	SetWindowExecutableIcon();
	applicationVersion = getVersion(devLaunch);
	updateWindowTitle();

	// Check if file to open was supplied
	std::string openFile = "";
	if (args.size() >= 2) {
		if (args[1] != "dev") {
			openFile = args[1];
		}
	}

	// Otherwise, open recent 
	if (openFile == "") {
		std::string file = readRecentFile();
		if (file != "") {
			openFile = file;
		}
	}

	// Load file
	if (openFile != "") {
		std::cout << "Loading file: " << openFile << std::endl;
	}

	// Setup was successful
	return true;
}

bool Application::Update() {
	using namespace Battery::Graphics;

	DrawBackground({ 255, 255, 255 });

	ImGui::Begin("Test");
	ImGui::End();

	//std::cout << "Framerate: " << framerate << std::endl;

	return true;
}

void Application::Shutdown() {
}

void Application::PostApplicationFunction() {
	updaterThread->join();
}

void Application::KeyPressed(int keycode, char unicode, int modifiers, bool repeat) {
	if (keycode == ALLEGRO_KEY_SPACE) {

		for (int i = 0; i < 100; i++) {
			float x1 = rand() % width;
			float y1 = rand() % height;
			float x2 = rand() % width;
			float y2 = rand() % height;

			//al_set_target_bitmap(bitmap);
			//renderer.DrawLine({ x1, y1 }, { x2, y2 }, (rand() % 2000) / 100.f, { rand() % 255, rand() % 255, rand() % 255 });
		}
	}
}








void Application::updateWindowTitle() {
	std::string title = filename + " - TechnicalSketcher " + applicationVersion;

	if (fileChanged)
		title = "*" + title;

	if (devLaunch)
		title += " - devLaunch";

	SetWindowTitle(title);
}

std::string Application::readRecentFile() {
	using namespace Battery::FileUtils;

	std::string path = GetAllegroStandardPath(ALLEGRO_USER_SETTINGS_PATH);
	path += RECENT_FILES_FILENAME;

	File file = LoadFile(path);

	if (file.fail())
		return "";

	return file.str();
}

bool Application::writeRecentFile(std::string content) {
	using namespace Battery::FileUtils;

	std::string path = GetAllegroStandardPath(ALLEGRO_USER_SETTINGS_PATH);
	path += RECENT_FILES_FILENAME;

	return SaveFile(path, content);
}
*/

#include "Battery/Battery.h"

class ImGuiLayer : public Battery::ImGuiLayer {
public:
	ImGuiLayer() {
	}

	~ImGuiLayer() {
	}

	void OnImGuiRender() override {
		ImGui::Begin("test");
		ImGui::End();
	}

	void OnImGuiEvent(Battery::Event* e) override {

		switch (e->GetType()) {
		case Battery::EventType::MouseButtonPressed:
		case Battery::EventType::MouseButtonReleased:
		case Battery::EventType::MouseMoved:
		case Battery::EventType::MouseScrolled:
			if (io.WantCaptureMouse) {
				LOG_ERROR("ImGui clicked");
				e->SetHandled();
				return;
			}
			break;
		}

		LOG_WARN("ImGui Event");
	}


};

class App : public Battery::Application {
public:
	App() : Battery::Application(800, 600, "MyApplication") {
		//LOG_SET_LOGLEVEL(BATTERY_LOG_LEVEL_INFO);
	}

	bool OnStartup() override {

		PushOverlay(new ImGuiLayer());

		return true;
	}

	void OnUpdate() override {
		//std::cout << framerate << std::endl;
	}

	void OnRender() override {
		Battery::Graphics::DrawBackground({ 0, 0, 0 });
	}

	void OnShutdown() override {

	}

	void OnEvent(Battery::Event* e) override {

		switch (e->GetType()) {

		case Battery::EventType::WindowClose:
			CloseApplication();
			e->SetHandled();
			break;

		default:
			break;
		}

		LOG_WARN("Base Event");
	}
};

Battery::Application* Battery::CreateApplication() {
	return new App();
}
