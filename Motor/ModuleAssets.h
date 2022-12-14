#ifndef __ModuleAssets_H__
#define __ModuleAssets_H__

#include "Module.h"
#include "Globals.h"
#include "Glew/include/glew.h"

#include "DevIL/include/il.h"
#include "DevIL/include/ilut.h"

#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"

#include <vector>
using namespace std;

#define MAX_MESHES 100
#define VERTEX_FEATURES 8

struct MeshData
{
	MeshData() {}

	~MeshData() {

		delete vertex;
		vertex = nullptr;

		delete index;
		index = nullptr;
	}

	uint id_index = 0; // index in VRAM
	uint num_index = 0;
	uint* index = nullptr;

	uint id_vertex = 0; // unique vertex in VRAM
	uint num_vertex = 0;
	float* vertex = nullptr;

	void DrawMesh();
};


class ModuleAssets : public Module
{
public:
	ModuleAssets(Application* app, bool start_enabled = true);
	~ModuleAssets();

	bool Init();
	update_status PreUpdate(float dt);
	update_status Update(float dt);
	update_status PostUpdate(float dt);
	bool CleanUp();

	void LoadFile(const char* filePath);
	void LoadMeshData(MeshData* mesh);

	// ------------------------------------ Draw
	void Render();

	bool wireframe = false;
	bool textureEnabled = true;

private:
	SDL_Event event;                        // Declare event handle
	char* dropped_filedir = "";                  // Pointer for directory of dropped file
	const char* assets_dir = "Assets/";

	MeshData* newMesh[MAX_MESHES];
	vector<MeshData*> meshList;

};
#endif