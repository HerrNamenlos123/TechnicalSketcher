#pragma once

#include "pch.h"
#include "imgui.h"
#include "imgui_impl_allegro5.h"


glm::vec3 color(float g);
glm::vec3 color(float r, float g, float b);
glm::vec4 color(float r, float g, float b, float a);
ALLEGRO_COLOR al_color(float r, float g, float b);
ALLEGRO_COLOR al_color(float r, float g, float b, float a);
ALLEGRO_COLOR al_color(glm::vec3 c);
ALLEGRO_COLOR al_color(glm::vec4 c);
ALLEGRO_COLOR al_color(glm::vec3 c, float a);


enum WINDOW_FLAGS {
	WINDOW_NOFLAGS = 0,
	WINDOW_MAXIMIZED = 1,
	WINDOW_MINIMIZED = 2,
	WINDOW_RESIZABLE = 4
};



class AllegroEngine {

	bool loaded = false;
	bool running = true;

	std::string defaultFont = "C:/Windows/Fonts/consola.ttf";

	glm::ivec2 mouseScroll = { 0, 0 };

public:
	ALLEGRO_DISPLAY* display = nullptr;
	ALLEGRO_FONT* font = nullptr;
	ALLEGRO_EVENT_QUEUE* events = nullptr;
	ALLEGRO_TIMER* timer = nullptr;

	// User accessible variables
	size_t framecount = 0;
	int width = 0;
	int height = 0;
	glm::vec2 pmouse = { 0, 0 };
	glm::vec2 mouse  = { 0, 0 };
	bool pmouseButtonLeftPressed = false;
	bool pmouseButtonRightPressed = false;
	bool pmouseWheelPressed = false;
	bool pmousePressed = false;
	bool mouseButtonLeftPressed = false;
	bool mouseButtonRightPressed = false;
	bool mouseWheelPressed = false;
	bool mousePressed = false;

	bool wantsToClose = false;



	// Constructor and deconstructor

	AllegroEngine();
	~AllegroEngine();



	// To be overridden by the user

	virtual void setup() {
	}

	virtual void draw() {
	}

	virtual void destroy() {
	}

	virtual void keyPressed(int keycode, unsigned int modifiers) {
	}

	virtual void keyPressed(int keycode, int unicode, unsigned int modifiers, bool repeat) {
	}

	virtual void keyReleased(int keycode, unsigned int modifiers) {
	}

	virtual void mouseScrolled(int x, int y) {
	}



	// Graphics utility

	void background(float c);
	void background(glm::vec3 c);

	void line(glm::vec2 p1, glm::vec2 p2, glm::vec3 color, float thickness);
	void fancyLine(glm::vec2 p1, glm::vec2 p2, glm::vec3 color, float thickness);

	void outlinedRectangle(glm::vec2 p1, glm::vec2 p2, glm::vec3 outlineColor, float outlineThickness);
	void filledRectangle(glm::vec2 p1, glm::vec2 p2, glm::vec3 color);
	void filledRectangle(glm::vec2 p1, glm::vec2 p2, glm::vec4 color);
	void outlinedFilledRectangle(glm::vec2 p1, glm::vec2 p2, glm::vec3 fillColor, glm::vec3 outlineColor, float outlineThickness);
	void outlinedFilledRectangleCentered(glm::vec2 center, glm::vec2 size, glm::vec3 fillColor, glm::vec3 outlineColor, float outlineThickness);



	// Engine utility

	bool getKey(int keycode);
	void close();
	void infoMessage(const std::string& str);
	void errorMessage(const std::string& str);




	// Main Engine functions

	void run(int w, int h, int flags = 0);		// Contains main loop
	bool initAll(int flags);
	void uninstallAll();

private:

	// Private functions

	void prepareFrame();
	void finishFrame();
};