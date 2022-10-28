#include "Globals.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "SDL\include\SDL_opengl.h"
#include <gl/GL.h>
#include <gl/GLU.h>


#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_opengl3.h"
#include "ImGui/imgui_impl_sdl.h"

#pragma comment (lib, "glu32.lib")    /* link OpenGL Utility lib     */
#pragma comment (lib, "opengl32.lib") /* link Microsoft OpenGL lib   */

ModuleRenderer3D::ModuleRenderer3D(Application* app, bool start_enabled) : Module(app, start_enabled)
{
}

// Destructor
ModuleRenderer3D::~ModuleRenderer3D()
{}

// Called before render is available
bool ModuleRenderer3D::Init()
{
	LOG("Creating 3D Renderer context");
	bool ret = true;
	
	//Create context
	context = SDL_GL_CreateContext(App->window->window);
	if(context == NULL)
	{
		LOG("OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	
	if(ret == true)
	{
		//Use Vsync
		if(VSYNC && SDL_GL_SetSwapInterval(1) < 0)
			LOG("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());

		//Initialize Projection Matrix
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		//Check for error
		GLenum error = glGetError();
		if(error != GL_NO_ERROR)
		{
			LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		//Initialize Modelview Matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		//Check for error
		error = glGetError();
		if(error != GL_NO_ERROR)
		{
			LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}
		
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glClearDepth(1.0f);
		
		//Initialize clear color
		glClearColor(0.f, 0.f, 0.f, 1.f);

		//Check for error
		error = glGetError();
		if(error != GL_NO_ERROR)
		{
			LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}
		
		GLfloat LightModelAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LightModelAmbient);
		
		lights[0].ref = GL_LIGHT0;
		lights[0].ambient.Set(0.25f, 0.25f, 0.25f, 1.0f);
		lights[0].diffuse.Set(0.75f, 0.75f, 0.75f, 1.0f);
		lights[0].SetPos(0.0f, 0.0f, 2.5f);
		lights[0].Init();
		
		GLfloat MaterialAmbient[] = {1.0f, 1.0f, 1.0f, 1.0f};
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, MaterialAmbient);

		GLfloat MaterialDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialDiffuse);
		
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		lights[0].Active(true);
		glEnable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
	}

	// Projection matrix for
	OnResize(SCREEN_WIDTH, SCREEN_HEIGHT);


	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(App->window->window, context);
	ImGui_ImplOpenGL3_Init("#version 130");

	return ret;
}

// PreUpdate: clear buffer
update_status ModuleRenderer3D::PreUpdate(float dt)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(App->camera->GetViewMatrix());

	// light 0 on cam pos
	lights[0].SetPos(App->camera->Position.x, App->camera->Position.y, App->camera->Position.z);

	for(uint i = 0; i < MAX_LIGHTS; ++i)
		lights[i].Render();

	return UPDATE_CONTINUE;
}

// PostUpdate present buffer to screen
update_status ModuleRenderer3D::PostUpdate(float dt)
{
	
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	
	
	if (docking == true) {
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
	}
	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	ImGui::ShowDemoWindow();
	ImGui::Begin("Main Scene", 0, ImGuiWindowFlags_MenuBar);
	ImGui::BeginChild("", ImVec2(SCREEN_WIDTH, SCREEN_HEIGHT));

	ImVec2 wsize = ImGui::GetWindowSize();

	ImGui::Image((ImTextureID)App->renderer3D->textColorBuff, wsize, ImVec2(0, 1), ImVec2(1, 0));

	ImGui::EndChild();
	ImGui::End();
	
	ImGui::BeginMenuBar();

	ImGui::Separator();

	if (ImGui::BeginMainMenuBar()) {

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.05f, 0.05f, 0.05f, 1));

		ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.95f, 0.95f, 0.95f, 1));

		if (ImGui::BeginMenu("File"))
		{

			if (ImGui::MenuItem("GitHub"))
			{
				ShellExecute(0, 0, "https://github.com/Astrorey776/Game-Engine", 0, 0, SW_SHOW);
			}

			if (ImGui::MenuItem("Exit"))
			{
				//Stop Button
				return UPDATE_STOP;
			}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Windows"))
		{
			
			ImGui::EndMenu();
		}

		if (pOpen_about)
		{
			if (ImGui::BeginMenu("INFO"))
			{
				ImGui::SetWindowSize(ImVec2(600.0f, 620.0f));

				//Title
				ImGui::Text("MM's ENGINE");

				//Description
				ImGui::Text("An engine attempt made by a student at the Centre de la Imatge i de la Tecnologia Multimedia (CITM)");
				ImGui::Text("");

				if (ImGui::Button("Docking", ImVec2(100, 20)))
				{
					
					docking = true;

				}

			    //Info about the engine
				ImGui::Text("License:");
				ImGui::Text("");
				ImGui::Text("MIT License");
				ImGui::Text("");
				ImGui::Text("Info about the Engine");
				ImGui::Text("");
				ImGui::Text("-Code on fbx drag & drop implemented but not functional.");
				ImGui::Text("-Configuration window");
				ImGui::Text("-Buttons to open github repository and close the program.");
				ImGui::Text("-Button to enable docking.");
				ImGui::EndMenu();
			}


			if (pOpen_about == NULL) aboutVisible = !aboutVisible; // Window is closed so function "MenuAbout()" stops being called
		}
		
		if (pOpen_config)
		{
			if (ImGui::Begin("Configuration", &pOpen_config))
			{
				//ImGui::SetWindowSize(ImVec2(450.0f, 300.0f));
				ImGui::TextColored(ImVec4(255,255, 255, 255), "OPTIONS");

				if (ImGui::CollapsingHeader("Window"))
				{
					if (ImGui::Checkbox("FullScreen", &fullscreen))
					{
						if (fullscreen)
						{
							SDL_SetWindowSize(App->window->window, 1920, 1080);
							SDL_SetWindowFullscreen(App->window->window, SDL_WINDOW_FULLSCREEN); //FULLSCREEN ENABLED
							

						}
						else
						{
							SDL_SetWindowSize(App->window->window, SCREEN_WIDTH, SCREEN_HEIGHT);
							SDL_SetWindowFullscreen(App->window->window, !SDL_WINDOW_FULLSCREEN); //FULLSCREEN DISABLED
							

						}
					}
					if (!fullscreen) //RESIZABLE, BORDER AND SIZE OPTIONS ONLY APPEARS IF IT IS IN WINDOW MODE
					{
						if (ImGui::Checkbox("Resizable", &resizable))
						{
							if (resizable)
							{
								SDL_SetWindowResizable(App->window->window, SDL_TRUE); //RESIZABLE ENABLED
								

							}
							else
							{
								SDL_SetWindowResizable(App->window->window, SDL_FALSE);//RESIZABLE DISABLED
								

							}
						}
						ImGui::SameLine();
						if (ImGui::Checkbox("Borderless", &borderless))
						{
							if (borderless)
							{
								SDL_SetWindowBordered(App->window->window, SDL_FALSE); //BORDERLESS ENABLED
								

							}
							else
							{
								SDL_SetWindowBordered(App->window->window, SDL_TRUE); //BORDERLESS DISABLED
								

							}
						}

						ImGui::Text("Width: ");
						ImGui::SameLine();
						ImGui::TextColored({ 0,255,0,255 }, "%d", SDL_GetWindowSurface(App->window->window)->w);

						ImGui::Text("Height: ");
						ImGui::SameLine();
						ImGui::TextColored({ 0,255,0,255 }, "%d", SDL_GetWindowSurface(App->window->window)->h);

					}
				}
				if (ImGui::CollapsingHeader("Visual"))
				{
					if (ImGui::Checkbox("Vsync", &vsync))
					{
						if (vsync)
						{
							SDL_GL_SetSwapInterval(1); //VSYNC ENABLED
							

						}
						else
						{
							SDL_GL_SetSwapInterval(0); //VSYNC DISABLED
							

						}
					}

					if (ImGui::SliderFloat("Brightness", &screenBrightness, 0.300f, 1.000f))
					{
						SDL_SetWindowBrightness(App->window->window, screenBrightness);
						

					}

					if (ImGui::Checkbox("Lights", &light))
					{

						if (light)
						{
							glEnable(GL_LIGHTING); //LIGHTS ENABLED
							

						}
						else
						{
							glDisable(GL_LIGHTING); //LIGHTS DISABLED
							
						}

					}
					ImGui::SameLine();
					if (ImGui::Checkbox("Depth Test", &depthTest))
					{
						if (depthTest)
						{
							glEnable(GL_DEPTH_TEST); //DEPTH TEST ENABLED
							

						}
						else
						{
							glDisable(GL_DEPTH_TEST); //DEPTH TEST DISABLED
							

						}
					}
					if (ImGui::Checkbox("Cull Face", &cullFace))
					{
						if (cullFace)
						{
							glEnable(GL_CULL_FACE); //CULL FACE ENABLED
							
						}
						else
						{
							glDisable(GL_CULL_FACE); //CULL FACE  DISABLED
							

						}
					}
					ImGui::SameLine();
					if (ImGui::Checkbox("Color Material", &colorMaterial))
					{
						if (colorMaterial)
						{
							glEnable(GL_COLOR_MATERIAL); //COLOR MATERIAL ENABLED
							

						}
						else
						{
							glDisable(GL_COLOR_MATERIAL); //COLOR MATERIAL  DISABLED
							

						}
					}
					ImGui::SameLine();
		

				}
				if (ImGui::CollapsingHeader("Hardware"))
				{

					ImGui::Text("-----");

					//SDL Version
					SDL_version version;
					SDL_GetVersion(&version);

					ImGui::Text("SDL Version:");
					ImGui::SameLine();
					ImGui::TextColored({ 255,255,0,20 }, "%u.%u.%u", version.major, version.minor, version.patch);

					ImGui::Text("-----");

					//CPU

					//Cache

					int SDL_GetCPUCount(void); //Aquest retorna el numero de nuclis del cpu.
					int SDL_GetCPUCacheLineSize(void); //Aquest retorna el tamany de la primera linea de cache de la CPU


					ImGui::Text("CPUs:");
					ImGui::SameLine();
					ImGui::TextColored({ 255,255,0,20 }, "%d", SDL_GetCPUCount());

					//Ram del sistema
					int SDL_GetSystemRAM(void); //Aquesta retorna la cantitat de ram de la que disposem

					ImGui::Text("System RAM:");
					ImGui::SameLine();
					ImGui::TextColored({ 255,255,0,20 }, "%dGb", SDL_GetSystemRAM() / 1000);

					//Caps--------------------------------
					ImGui::Text("Caps:");

					//Initiation of parameters

					SDL_bool SDL_Has3DNow(void);
					SDL_bool SDL_HasAltiVec(void);
					SDL_bool SDL_HasAVX(void);
					SDL_bool SDL_HasAVX2(void);
					SDL_bool SDL_HasMMX(void);
					SDL_bool SDL_HasRDTSC(void);
					SDL_bool SDL_HasSSE(void);
					SDL_bool SDL_HasSSE2(void);
					SDL_bool SDL_HasSSE3(void);
					SDL_bool SDL_HasSSE41(void);
					SDL_bool SDL_HasSSE42(void);

					//3dNow


					if (SDL_Has3DNow())
					{
						ImGui::SameLine();
						ImGui::TextColored({ 255,255,0,20 }, "3DNow!");
					}

					//AltiVec

					if (SDL_HasAltiVec())
					{
						ImGui::SameLine();
						ImGui::TextColored({ 255,255,0,20 }, "AltiVec");
					}


					//AVX
					if (SDL_HasAVX())
					{
						ImGui::SameLine();
						ImGui::TextColored({ 255,255,0,20 }, "AVX");
					}

					//AVX2
					if (SDL_HasAVX2())
					{
						ImGui::SameLine();
						ImGui::TextColored({ 255,255,0,20 }, "AVX2");
					}
					//MMX
					if (SDL_HasMMX())
					{
						ImGui::SameLine();
						ImGui::TextColored({ 255,255,0,20 }, "MMX");
					}
					//RDTSC
					if (SDL_HasRDTSC())
					{
						ImGui::SameLine();
						ImGui::TextColored({ 255,255,0,20 }, "RDTSC");
					}
					//SSE
					if (SDL_HasSSE())
					{
						ImGui::SameLine();
						ImGui::TextColored({ 255,255,0,20 }, "SSE");
					}
					//SSE2
					if (SDL_HasSSE2())
					{
						ImGui::SameLine();
						ImGui::TextColored({ 255,255,0,20 }, "SSE2");
					}
					//SSE3
					if (SDL_HasSSE3())
					{
						ImGui::SameLine();
						ImGui::TextColored({ 255,255,0,20 }, "SSE3");
					}
					//SSE41
					if (SDL_HasSSE41())
					{
						ImGui::SameLine();
						ImGui::TextColored({ 255,255,0,20 }, "SSE41");
					}
					//SSE42
					if (SDL_HasSSE42())
					{
						ImGui::SameLine();
						ImGui::TextColored({ 255,255,0,20 }, "SSE42");
					}

					ImGui::Text("-----");

					//GPU
					const char* SDL_GetCurrentVideoDriver(void);
					ImGui::Text("Brand:");
					ImGui::SameLine();
					ImGui::TextColored({ 255,255,0,20 }, "%s", SDL_GetCurrentVideoDriver());

				}
			}

			ImGui::End();

			if (pOpen_config == NULL) configVisible = !configVisible; // Window is closed so function "MenuConfig()" stops being called
		}
		ImGui::PopStyleColor(2);

		ImGui::EndMainMenuBar();

		

		

	}

	

	/*ImGui::EndMenuBar();
	ImGui::BeginChild("", ImVec2(SCREEN_WIDTH, SCREEN_HEIGHT));ImGui::EndChild();*/
	
	


	ImGuiIO& io = ImGui::GetIO(); (void)io;


	// Rendering
	ImGui::Render();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	//glClearColor(1.0, 1.0, 1.0, 0.0);
	//glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	SDL_GL_SwapWindow(App->window->window);
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleRenderer3D::CleanUp()
{
	LOG("Destroying 3D Renderer");


	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(context);

	return true;
}


void ModuleRenderer3D::OnResize(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	ProjectionMatrix = perspective(60.0f, (float)width / (float)height, 0.125f, 512.0f);
	glLoadMatrixf(&ProjectionMatrix);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
