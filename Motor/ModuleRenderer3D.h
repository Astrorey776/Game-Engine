#pragma once
#include "Module.h"
#include "Globals.h"
#include "glmath.h"
#include "Light.h"

#define MAX_LIGHTS 8

class ModuleRenderer3D : public Module
{
public:
	ModuleRenderer3D(Application* app, bool start_enabled = true);
	~ModuleRenderer3D();

	bool Init();
	update_status PreUpdate(float dt);
	update_status PostUpdate(float dt);
	bool CleanUp();

	void OnResize(int width, int height);

public:
	uint textureColorbuffer;

	Light lights[MAX_LIGHTS];
	SDL_GLContext context;
	mat3x3 NormalMatrix;
	mat4x4 ModelMatrix, ViewMatrix, ProjectionMatrix;

	bool pOpen_config = true,
		pOpen_about = true;
		

	uint framebuffer = 0;
	uint rbo;
	unsigned int textColorBuff;

	bool docking = false;

	bool vsync = VSYNC;

private:

	//Configuration window
	// 
//window
	bool fullscreen = WIN_FULLSCREEN;
	bool resizable = WIN_RESIZABLE;
	bool borderless = WIN_BORDERLESS;
	bool fullDesk = WIN_FULLSCREEN_DESKTOP;
	int screenWidth = SCREEN_WIDTH;
	int screenHeight = SCREEN_HEIGHT;

	//Screen
	float screenBrightness = 1;
	bool light = true;
	bool depthTest = true;
	bool cullFace = true;
	bool colorMaterial = true;


	bool configVisible = true;
	bool aboutVisible = false;

};