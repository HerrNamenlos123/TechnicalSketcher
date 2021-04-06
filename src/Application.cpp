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





#include "pch.h"
#include "Application.h"
#include "Gui.h"
#include "Updater.h"
#include "NavigatorLayer.h"
#include "../TechnicalSketcher/resource.h"	// For the icon

#include "Layer.h"

App::App() : Battery::Application(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, APPLICATION_NAME) {
	//LOG_SET_LOGLEVEL(BATTERY_LOG_LEVEL_TRACE);
	SetWindowFlags(ALLEGRO_RESIZABLE);
}

bool App::OnStartup() {

	// Set the window size and position
	glm::vec2 monitorSize = GetPrimaryMonitorSize();
	float margin = 1.2f;					// If screen is too small with a little margin, maximize the window
	if (monitorSize.x / margin < DEFAULT_WINDOW_WIDTH || monitorSize.y / margin < DEFAULT_WINDOW_HEIGHT) {
		window.SetSize(monitorSize / margin);
		window.SetScreenPosition({ 0, 0 });
		window.Maximize();
	}
	else {	// Set up window normally
		window.SetScreenPosition((monitorSize - glm::vec2(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT)) / 2.f);
		window.SetSize({ DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT });
	}

	// Initialize the renderer
	ApplicationRenderer::Load();

	// Various layers
	PushLayer(new UpdaterLayer());
	PushLayer(new NavigatorLayer());
	PushOverlay(new GUI::GuiLayer());

	// Set the icon and title of the window
	window.SetWindowExecutableIcon();
	window.SetTitle(APPLICATION_NAME);

	/*
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
	}*/

	return true;
}

void App::OnUpdate() {
	using namespace Battery;

	// Only refresh the screen eventually to save cpu power
	// Allow the first 60 frames to let everything initialize
	// and only if the program runs in the background
	if (TimeUtils::GetRuntime() < (lastScreenRefresh + passiveScreenTime) && 
		framecount > 60 && !window.IsFocused()) 
	{
		DiscardFrame();
		return;
	}
	else {
		lastScreenRefresh = TimeUtils::GetRuntime();
	}
}

void App::OnRender() {
	using namespace Battery;
}

void App::OnShutdown() {
	// Unload the renderer
	ApplicationRenderer::Unload();
}

void App::OnEvent(Battery::Event* e) {
	using namespace Battery;

	// Force a screen update
	lastScreenRefresh = 0;

	// Act on the event
	switch (e->GetType()) {

	case EventType::KeyPressed:

		if (static_cast<KeyPressedEvent*>(e)->keycode == ALLEGRO_KEY_SPACE) {
			e->SetHandled();
			//Navigator::GetInstance()->PrintShapes();
			//LOG_ERROR("{}", Navigator::GetInstance()->file.GetJson().dump(4));
			//Battery::FileUtils::LoadFileWithDialog("txt", window);
		}
		break;

	case EventType::WindowClose:
		Navigator::GetInstance()->CloseApplication();
		e->SetHandled();
		break;

	default:
		break;
	}
}

Battery::Application* Battery::CreateApplication() {
	return new App();
}

App* GetClientApplication() {
	return static_cast<App*>(Battery::Application::GetApplicationPointer());
}








/*


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
	}


};

class App : public Battery::Application {
public:

	Battery::Scene* scene = nullptr;
	Battery::ShaderProgram* shader = nullptr;

	glm::vec2 p1 = { 100, 100 };
	glm::vec2 p2 = { 300, 300 };
	float thickness = 5;
	float falloff = 1;
	glm::vec4 color = { 0, 0, 0, 255 };

	App() : Battery::Application(800, 600, "MyApplication") {
		//LOG_SET_LOGLEVEL(BATTERY_LOG_LEVEL_TRACE);
	}

	bool OnStartup() override {
		using namespace Battery;

		scene = new Scene(&window);
		shader = new ShaderProgram(window.allegroDisplayPointer, "../resource/vertex.glsl", "../resource/fragment.glsl");

		scene->textures.push_back(Texture2D("../resource/wood-abstract-crack-nature-pattern-sample-textures.jpg",
			ALLEGRO_MAG_LINEAR | ALLEGRO_MIN_LINEAR));

		//PushOverlay(new ImGuiLayer());

		return true;
	}

	void OnUpdate() override {
		using namespace Battery;

		float speed1 = 5;
		float speed2 = 1;
		float speed3 = 1;

		if (GetKey(ALLEGRO_KEY_W)) {
			p1.y -= speed1;
		}
		if (GetKey(ALLEGRO_KEY_S)) {
			p1.y += speed1;
		}
		if (GetKey(ALLEGRO_KEY_A)) {
			p1.x -= speed1;
		}
		if (GetKey(ALLEGRO_KEY_D)) {
			p1.x += speed1;
		}

		if (GetKey(ALLEGRO_KEY_UP)) {
			p2.y -= speed1;
		}
		if (GetKey(ALLEGRO_KEY_DOWN)) {
			p2.y += speed1;
		}
		if (GetKey(ALLEGRO_KEY_LEFT)) {
			p2.x -= speed1;
		}
		if (GetKey(ALLEGRO_KEY_RIGHT)) {
			p2.x += speed1;
		}

		if (GetKey(ALLEGRO_KEY_I)) {
			thickness += speed2;
		}
		if (GetKey(ALLEGRO_KEY_K)) {
			thickness -= speed2;
		}

		if (GetKey(ALLEGRO_KEY_O)) {
			falloff += speed2;
		}
		if (GetKey(ALLEGRO_KEY_L)) {
			falloff -= speed2;
		}

		if (GetKey(ALLEGRO_KEY_R)) {
			color.x += speed3;
		}
		if (GetKey(ALLEGRO_KEY_F)) {
			color.x -= speed3;
		}
		if (GetKey(ALLEGRO_KEY_T)) {
			color.y += speed3;
		}
		if (GetKey(ALLEGRO_KEY_G)) {
			color.y -= speed3;
		}
		if (GetKey(ALLEGRO_KEY_Z)) {
			color.z += speed3;
		}
		if (GetKey(ALLEGRO_KEY_H)) {
			color.z -= speed3;
		}
		if (GetKey(ALLEGRO_KEY_U)) {
			color.w += speed3;
		}
		if (GetKey(ALLEGRO_KEY_J)) {
			color.w -= speed3;
		}
	}

	void OnRender() override {
		using namespace Battery;
		Renderer2D::BeginScene(scene);

		Renderer2D::BeginQuads(shader, 0);

		VertexData v1 = { glm::vec3(100, 100, 0), glm::vec2(0, 0), glm::vec4(255, 0, 0, 255) };
		VertexData v2 = { glm::vec3(300, 100, 0), glm::vec2(1, 0), glm::vec4(0, 255, 0, 255) };
		VertexData v3 = { glm::vec3(300, 300, 0), glm::vec2(1, 1), glm::vec4(0, 0, 255, 255) };
		VertexData v4 = { glm::vec3(100, 300, 0), glm::vec2(0, 1), glm::vec4(255, 0, 255, 255) };

		VertexData v5 = { glm::vec3(400, 400, 0), glm::vec2(0, 0), glm::vec4(255, 0, 0, 255) };
		VertexData v6 = { glm::vec3(500, 400, 0), glm::vec2(1, 0), glm::vec4(0, 255, 0, 255) };
		VertexData v7 = { glm::vec3(500, 500, 0), glm::vec2(1, 1), glm::vec4(0, 0, 255, 255) };
		VertexData v8 = { glm::vec3(400, 500, 0), glm::vec2(0, 1), glm::vec4(255, 0, 255, 255) };

		Renderer2D::DrawQuad(v1, v2, v3, v4);
		Renderer2D::DrawQuad(v5, v6, v7, v8);

		Renderer2D::EndQuads();

		Renderer2D::BeginCircles();
		Renderer2D::DrawCircle({ p1.x, p1.y }, thickness, color, falloff);
		Renderer2D::EndCircles();

		Renderer2D::EndScene();
	}

	void OnShutdown() override {
		delete shader;
		delete scene;
	}

	void OnEvent(Battery::Event* e) override {
		using namespace Battery;

		switch (e->GetType()) {

		case EventType::TextInput:
			//LOG_INFO(static_cast<TextInputEvent*>(e)->character);
			break;

		case EventType::KeyPressed:
			if (static_cast<KeyPressedEvent*>(e)->keycode == ALLEGRO_KEY_ESCAPE) {
				CloseApplication();
				e->SetHandled();
			}
			else if (static_cast<KeyPressedEvent*>(e)->keycode == ALLEGRO_KEY_SPACE) {
				scene->textures[0].SetFlags(0);
			}
			else if (static_cast<KeyPressedEvent*>(e)->keycode == ALLEGRO_KEY_B) {
				scene->textures[0].SetFlags(ALLEGRO_MAG_LINEAR | ALLEGRO_MIN_LINEAR);
			}
			break;

		case EventType::WindowClose:
			CloseApplication();
			e->SetHandled();
			break;

		default:
			break;
		}
	}
};

Battery::Application* Battery::CreateApplication() {
	return new App();
}

*/



// LADEKREIS!!!!!!!!!!!!!

// (arc shape)

// float speed = 8;
// float pulseSpeed = 7;
// float offset = 180;
// start += speed + pow(sin(count / 1000 * pulseSpeed), 2) * 5;
// end = start + 45 + pow(sin(count / 1000 * pulseSpeed + offset), 2) * 270;



// float speed = 200;
// float pulseSpeed = 3;
// float turnDegrees = 310;
// float startPulseAmount = 300;
// float time = TimeUtils::GetRuntime();
// start += (speed + pow(sin(time * pulseSpeed), 2) * startPulseAmount) * frametime;
// end = start + (360 - turnDegrees) / 2 + pow(sin(time * pulseSpeed + 180), 2) * turnDegrees;