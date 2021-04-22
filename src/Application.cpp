
#include "pch.h"
#include "Application.h"
#include "Gui.h"
#include "Updater.h"
#include "NavigatorLayer.h"
#include "../TechnicalSketcher/resource.h"	// For the icon

#include "Layer.h"

App::App() : Battery::Application(SPLASH_SCREEN_WIDTH, SPLASH_SCREEN_HEIGHT, APPLICATION_NAME) {
	//LOG_SET_LOGLEVEL(BATTERY_LOG_LEVEL_TRACE);
	SetWindowFlags(ALLEGRO_RESIZABLE | ALLEGRO_FRAMELESS);
}

bool App::OnStartup() {

	// Show splash screen
	Battery::Texture2D splash;
	splash.LoadEmbeddedResource(IDB_PNG1);
	if (splash.IsValid()) {
		al_draw_bitmap(splash.GetAllegroBitmap(), 0, 0, 0);
		al_flip_display();
	}

	// Initialize the renderer
	ApplicationRenderer::Load();

	// Various layers
	PushLayer(new NavigatorLayer());
	PushOverlay(new GUI::GuiLayer());

	// Load the settings
	if (!Navigator::GetInstance()->LoadSettings()) {
		Navigator::GetInstance()->SaveSettings();	// If settings can't be loaded, save the default settings
	}

	// Now that settings are loaded, start updater
	PushLayer(new UpdaterLayer());

	// Set the icon and title of the window
	window.SetWindowExecutableIcon();
	window.SetTitle(APPLICATION_NAME);

	// Check if file to open was supplied
	std::string openFile = "";
	bool newFile = false;
	if (args.size() >= 2) {
		if (args[1] == "new") {
			newFile = true;
		}

		openFile = args[1];
	}

	// Otherwise, open most recent 
	if (openFile == "") {
		openFile = Navigator::GetInstance()->GetMostRecentFile();
	}

	// Now open it
	if (openFile != "" && !newFile) {
		Navigator::GetInstance()->OpenFile(openFile);
	}

	// Set the window size and position
	Battery::Renderer2D::DrawBackground({ 255, 255, 255, 255 });
	al_flip_display();
	glm::vec2 monitorSize = GetPrimaryMonitorSize();	// If screen is too small with a little margin, maximize the window				
	if (monitorSize.x / SCREEN_SIZE_MARGIN < DEFAULT_WINDOW_WIDTH || monitorSize.y / SCREEN_SIZE_MARGIN < DEFAULT_WINDOW_HEIGHT) {
		window.SetSize(monitorSize / SCREEN_SIZE_MARGIN);
		window.SetScreenPosition((monitorSize - glm::vec2(window.GetSize())) / 2.f + glm::vec2(0, -30));
		window.Maximize();
	}
	else {	// Set up window normally
		window.SetScreenPosition((monitorSize - glm::vec2(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT)) / 2.f + glm::vec2(0, -30));
		window.SetSize({ DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT });
	}

	// Show the window in the taskbar
	long style = GetWindowLong(window.GetWinHandle(), GWL_STYLE);
	style |= WS_VISIBLE;
	ShowWindow(window.GetWinHandle(), SW_HIDE);
	SetWindowLong(window.GetWinHandle(), GWL_STYLE, style);

	// Setup display
	al_set_display_flag(window.allegroDisplayPointer, ALLEGRO_FRAMELESS, false);
	Battery::Renderer2D::DrawBackground({ 255, 255, 255, 255 });
	al_flip_display();
	ShowWindow(window.GetWinHandle(), SW_SHOW);

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

		//if (static_cast<KeyPressedEvent*>(e)->keycode == ALLEGRO_KEY_SPACE) {
		//	e->SetHandled();
		//	//Battery::Texture2D image = Navigator::GetInstance()->file.ExportImage();
		//	//window.SetClipboardImage(image);
		//	//image.SaveImage("test.png");
		//	//system("start mspaint.exe test.png");
		//
		//	//Battery::Texture2D image = Navigator::GetInstance()->file.ExportImage();
		//	//window.SetClipboardImage(image);
		//	//image.SaveImage("test.png");
		//
		//	//system("start \"C:\\Program Files\\GIMP 2\\bin\\gimp-2.10.exe\" test.png");
		//}

		if (static_cast<KeyPressedEvent*>(e)->keycode == 'h') {
			e->SetHandled();


		}

		if (static_cast<KeyPressedEvent*>(e)->keycode == 'j') {
			e->SetHandled();

			//long style = GetWindowLong(hWnd, GWL_STYLE);
			//style &= ~(WS_VISIBLE);    // this works - window become invisible 
			//
			//style |= WS_EX_TOOLWINDOW;   // flags don't work - windows remains in taskbar
			//style &= ~(WS_EX_APPWINDOW);
			//
			//ShowWindow(hWnd, SW_HIDE); // hide the window
			//SetWindowLong(hWnd, GWL_STYLE, style); // set the style
			//ShowWindow(hWnd, SW_SHOW); // show the window for the new style to come into effect
			//ShowWindow(hWnd, SW_HIDE); // hide the window so we can't see it
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