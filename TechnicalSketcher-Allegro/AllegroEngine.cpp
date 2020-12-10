
#include "pch.h"
#include "AllegroEngine.h"




glm::vec3 color(float g) {
	return { g, g, g };
}

glm::vec3 color(float r, float g, float b) {
	return { r, g, b };
}

glm::vec4 color(float r, float g, float b, float a) {
	return { r, g, b, a };
}

ALLEGRO_COLOR al_color(float r, float g, float b) {
	return al_map_rgb(r, g, b);
}

ALLEGRO_COLOR al_color(float r, float g, float b, float a) {
	return al_map_rgba(r, g, b, a);
}

ALLEGRO_COLOR al_color(glm::vec3 c) {
	return al_map_rgb(c.x, c.y, c.z);
}

ALLEGRO_COLOR al_color(glm::vec4 c) {
	return al_map_rgba(c.x, c.y, c.z, c.w);
}

ALLEGRO_COLOR al_color(glm::vec3 c, float a) {
	return al_map_rgba(c.x, c.y, c.z, a);
}




AllegroEngine::AllegroEngine() {
}

AllegroEngine::~AllegroEngine() {
	uninstallAll();
}







// Graphics utility

void AllegroEngine::background(float c) {
	background({ c, c, c });
}

void AllegroEngine::background(glm::vec3 c) {
	al_clear_to_color(al_color(c));
}

void AllegroEngine::line(glm::vec2 p1, glm::vec2 p2, glm::vec3 color, float thickness) {
	al_draw_line(p1.x, p1.y, p2.x, p2.y, al_color(color), thickness);
}

void AllegroEngine::fancyLine(glm::vec2 p1, glm::vec2 p2, glm::vec3 color, float thickness) {
	al_draw_line(p1.x, p1.y, p2.x, p2.y, al_color(color), thickness);
	al_draw_filled_circle(p1.x, p1.y, thickness / 2.f, al_color(color));
	al_draw_filled_circle(p2.x, p2.y, thickness / 2.f, al_color(color));
}

void AllegroEngine::outlinedRectangle(glm::vec2 p1, glm::vec2 p2, glm::vec3 outlineColor, float outlineThickness) {
	al_draw_rectangle(p1.x, p1.y, p2.x, p2.y, al_color(outlineColor), outlineThickness);
}

void AllegroEngine::filledRectangle(glm::vec2 p1, glm::vec2 p2, glm::vec3 color) {
	al_draw_filled_rectangle(p1.x, p1.y, p2.x, p2.y, al_color(color));
}

void AllegroEngine::filledRectangle(glm::vec2 p1, glm::vec2 p2, glm::vec4 color) {
	al_draw_filled_rectangle(p1.x, p1.y, p2.x, p2.y, al_color(color));
}

void AllegroEngine::outlinedFilledRectangle(glm::vec2 p1, glm::vec2 p2, glm::vec3 fillColor, glm::vec3 outlineColor, float outlineThickness) {
	filledRectangle(p1, p2, fillColor);
	outlinedRectangle(p1, p2, outlineColor, outlineThickness);
}

void AllegroEngine::outlinedFilledRectangleCentered(glm::vec2 center, glm::vec2 size, glm::vec3 fillColor, glm::vec3 outlineColor, float outlineThickness) {
	filledRectangle(center - size / 2.f, center + size / 2.f, fillColor);
	outlinedRectangle(center - size / 2.f, center + size / 2.f, outlineColor, outlineThickness);
}





// Engine utility

bool AllegroEngine::getKey(int keycode) {
	ALLEGRO_KEYBOARD_STATE keyboardState = ALLEGRO_KEYBOARD_STATE();

	al_get_keyboard_state(&keyboardState);

	return al_key_down(&keyboardState, keycode);
}

void AllegroEngine::close() {
	running = false;
}

void AllegroEngine::errorMessage(const std::string& str) {
	al_show_native_message_box(display, "Error!", str.c_str(), NULL, NULL, ALLEGRO_MESSAGEBOX_ERROR);
}








// Main Engine functions

void AllegroEngine::run(int w, int h, int flags) {

	width = w;
	height = h;

	// Initialize everything
	if (!initAll(flags)) {
		uninstallAll();
		return;
	}

	// Add event triggers
	al_register_event_source(events, al_get_keyboard_event_source());
	al_register_event_source(events, al_get_mouse_event_source());
	al_register_event_source(events, al_get_display_event_source(display));
	al_register_event_source(events, al_get_timer_event_source(timer));

	// Call user overridden setup function
	setup();

	// Start FPS timer
	al_start_timer(timer);

	bool drawn = false;
	ALLEGRO_EVENT event;
	while (running) {

		if (!drawn) {
			prepareFrame();
			draw();
			finishFrame();
			drawn = true;
		}

		// Wait until any event happens
		al_wait_for_event(events, &event);

		// Let ImGui know about any event
		ImGui_ImplAllegro5_ProcessEvent(&event);

		switch (event.type) {

		case ALLEGRO_EVENT_KEY_DOWN:
			keyPressed(event.keyboard.keycode, event.keyboard.modifiers);
			break;

		case ALLEGRO_EVENT_KEY_UP:
			keyReleased(event.keyboard.keycode, event.keyboard.modifiers);
			break;

		case ALLEGRO_EVENT_KEY_CHAR:
			keyPressed(event.keyboard.keycode, event.keyboard.unichar, event.keyboard.modifiers, event.keyboard.repeat);
			break;

		case ALLEGRO_EVENT_DISPLAY_CLOSE:
			close();
			break;

		case ALLEGRO_EVENT_DISPLAY_RESIZE:
			ImGui_ImplAllegro5_InvalidateDeviceObjects();
			al_acknowledge_resize(display);
			ImGui_ImplAllegro5_CreateDeviceObjects();
			break;

		case ALLEGRO_EVENT_TIMER:
			al_flip_display();
			drawn = false;
			break;

		default:
			break;
		}
	}

	// Call user overridden destroy function
	destroy();

	// Finally uninstall all installed modules
	uninstallAll();

}

bool AllegroEngine::initAll(int flags) {

	al_init();

	if (!al_init_font_addon()) {
		errorMessage("Failed to initialize Allegro font addon!");
		return false;
	}

	if (!al_init_ttf_addon()) {
		errorMessage("Failed to initialize Allegro ttf addon!");
		return false;
	}

	if (!al_init_primitives_addon()) {
		errorMessage("Failed to initialize Allegro primitives addon!");
		return false;
	}

	if (!al_init_image_addon()) {
		errorMessage("Failed to initialize Allegro primitives addon!");
		return false;
	}

	if (flags & WINDOW_RESIZABLE)
		al_set_new_display_flags(ALLEGRO_RESIZABLE);

	if (flags & WINDOW_MINIMIZED)
		al_set_new_display_flags(ALLEGRO_MINIMIZED);

	if (flags & WINDOW_MAXIMIZED)
		al_set_new_display_flags(ALLEGRO_MAXIMIZED);

	al_convert_memory_bitmaps();

	display = al_create_display(width, height);
	if (display == nullptr) {
		errorMessage("Failed to create Allegro Window!");
		return false;
	}

	font = al_load_ttf_font(defaultFont.c_str(), 64, 0);
	if (font == nullptr) {
		errorMessage("Failed to load TTF file " + defaultFont);
		return false;
	}

	events = al_create_event_queue();
	if (events == nullptr) {
		errorMessage("Failed to create Allegro event queue!");
		return false;
	}

	timer = al_create_timer(1.f / 60.f);
	if (timer == nullptr) {
		errorMessage("Failed to create Allegro timer!");
		return false;
	}

	if (!al_install_keyboard()) {
		errorMessage("Failed to install Allegro keyboard!");
		return false;
	}

	if (!al_install_mouse()) {
		errorMessage("Failed to install Allegro mouse!");
		return false;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplAllegro5_Init(display);

	return true;
}

void AllegroEngine::uninstallAll() {

	ImGui_ImplAllegro5_Shutdown();
	//ImGui::DestroyContext();

	if (display != nullptr) {
		al_destroy_display(display);
		display = nullptr;
	}

	if (font != nullptr) {
		al_destroy_font(font);
		font = nullptr;
	}

	if (timer != nullptr) {
		al_destroy_timer(timer);
		timer = nullptr;
	}

	al_uninstall_keyboard();
	al_uninstall_mouse();
}








// Private

void AllegroEngine::prepareFrame() {
	width = al_get_display_width(display);
	height = al_get_display_height(display);

	framecount++;

	ALLEGRO_MOUSE_STATE state;
	al_get_mouse_state(&state);
	pmouse = mouse;
	mouse = { state.x, state.y };
	glm::ivec2 scrolled = mouseScroll - glm::ivec2(state.w, state.z);
	mouseScroll = glm::ivec2(state.w, state.z);
	if (scrolled.x != 0 || scrolled.y != 0)
		mouseScrolled(scrolled.x, scrolled.y);

	pmouseButtonLeftPressed = mouseButtonLeftPressed;
	pmouseButtonRightPressed = mouseButtonRightPressed;
	pmouseWheelPressed = mouseWheelPressed;
	pmousePressed = mousePressed;

	mouseButtonLeftPressed = state.buttons & 1;
	mouseButtonRightPressed = state.buttons & 2;
	mouseWheelPressed = state.buttons & 4;
	mousePressed = mouseButtonLeftPressed || mouseButtonRightPressed || mouseWheelPressed;

	// Start ImGui Frame
	ImGui_ImplAllegro5_NewFrame();
	ImGui::NewFrame();
}

void AllegroEngine::finishFrame() {
	ImGui::Render();
	ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());
	al_flip_display();
}