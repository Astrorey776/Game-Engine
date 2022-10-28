#include "Application.h"
#include "ModuleAssets.h"



namespace fs = std::filesystem;

ModuleAssets::ModuleAssets(Application* app, bool start_enabled) : Module(app, start_enabled)
{
}

// Destructor
ModuleAssets::~ModuleAssets()
{}

// Called before render is available
bool ModuleAssets::Init()
{
	bool ret = true;

	// Stream log messages to Debug window
	struct aiLogStream stream;
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
	aiAttachLogStream(&stream);

	// ------------------------------------- Load All Existing .fbx files -------------------------------------
	std::string path = "Assets/";
	for (const auto& entry : fs::directory_iterator(path))
	{
		// We get the FileName and Extension
		string fileName = entry.path().filename().string();
		const char* fileName_char = fileName.c_str();

		char existent_filedir[100];

		strcpy_s(existent_filedir, assets_dir);
		strcat_s(existent_filedir, fileName_char);

		// We get the Extension
		string extension = fs::path(fileName_char).extension().string();
		const char* extension_char = extension.c_str();

		if (existent_filedir != nullptr && extension == ".fbx")
		{
			LoadFile(existent_filedir);
		}
		

	}

	return ret;
}

update_status ModuleAssets::PreUpdate(float dt)
{

	return UPDATE_CONTINUE;
}

update_status ModuleAssets::Update(float dt)
{
	// ------------------------------------------------------------------------ Drag & Drop LOGIC
	SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case (SDL_DROPFILE): {      // In case if dropped file
			dropped_filedir = event.drop.file;

			// We get the FileName and Extension
			string fileName = fs::path(dropped_filedir).filename().string();
			const char* fileName_char = fileName.c_str();

			char new_filedir[100];

			strcpy_s(new_filedir, assets_dir);
			strcat_s(new_filedir, fileName_char);

			// We copy the dropped file to "Assets/" dir, with its name
			CopyFile(dropped_filedir, new_filedir, FALSE);

			// We get the Extension
			string extension = fs::path(dropped_filedir).extension().string();
			const char* extension_char = extension.c_str();

			if (extension == ".fbx" && new_filedir != nullptr)
			{
				LoadFile(new_filedir);
			}

		}
						   SDL_free(dropped_filedir);    // Free dropped_filedir memory

						   break;
		}

	}
	return UPDATE_CONTINUE;
}

// PostUpdate present buffer to screen
update_status ModuleAssets::PostUpdate(float dt)
{

	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleAssets::CleanUp()
{
	//LOG("Destroying Module");
	

	aiDetachAllLogStreams();

	//newMesh->~MeshData();
	for (int m = 0; m < meshList.size(); m++)
	{
		delete newMesh[m];
		newMesh[m] = nullptr;
	}


	meshList.clear();

	return true;
}

void ModuleAssets::LoadFile(const char* file_path)
{
	const aiScene* scene = aiImportFile(file_path, aiProcessPreset_TargetRealtime_MaxQuality);

	if (scene != nullptr && scene->HasMeshes())
	{
		for (int i = 0; i < scene->mNumMeshes; i++)
		{
			int m = meshList.size();

			newMesh[m] = new MeshData();
			// copy vertices
			newMesh[m]->num_vertex = scene->mMeshes[i]->mNumVertices;
			newMesh[m]->vertex = new float[newMesh[m]->num_vertex * VERTEX_FEATURES];
			//memcpy(newMesh->vertex, scene->mMeshes[i]->mVertices, sizeof(float) * newMesh->num_vertex * 3)

			for (int v = 0; v < newMesh[m]->num_vertex; v++) {
				// Vertex
				newMesh[m]->vertex[v * VERTEX_FEATURES] = scene->mMeshes[i]->mVertices[v].x;
				newMesh[m]->vertex[v * VERTEX_FEATURES + 1] = scene->mMeshes[i]->mVertices[v].y;
				newMesh[m]->vertex[v * VERTEX_FEATURES + 2] = scene->mMeshes[i]->mVertices[v].z;

				if (scene->mMeshes[i]->HasTextureCoords(0))
				{
					// UVs
					newMesh[m]->vertex[v * VERTEX_FEATURES + 3] = scene->mMeshes[i]->mTextureCoords[0][v].x;
					newMesh[m]->vertex[v * VERTEX_FEATURES + 4] = scene->mMeshes[i]->mTextureCoords[0][v].y;
				}
				// -------------------------------------------------------------------------------------- In a future
				if (scene->mMeshes[i]->HasNormals())
				{
					newMesh[m]->vertex[v * VERTEX_FEATURES + 5] = scene->mMeshes[i]->mNormals[v].x;
					newMesh[m]->vertex[v * VERTEX_FEATURES + 6] = scene->mMeshes[i]->mNormals[v].y;
					newMesh[m]->vertex[v * VERTEX_FEATURES + 7] = scene->mMeshes[i]->mNormals[v].z;
				}
			}

			// copy faces
			if (scene->mMeshes[i]->HasFaces())
			{
				newMesh[m]->num_index = scene->mMeshes[i]->mNumFaces * 3;
				newMesh[m]->index = new uint[newMesh[m]->num_index]; // assume each face is a triangle

				for (uint j = 0; j < scene->mMeshes[i]->mNumFaces; ++j)
				{
					if (scene->mMeshes[i]->mFaces[j].mNumIndices != 3)
					{
					}
					else
					{
						memcpy(&newMesh[m]->index[j * 3], scene->mMeshes[i]->mFaces[j].mIndices, 3 * sizeof(uint));
					}
				}
				LoadMeshData(newMesh[m]);
			}
			else {
				delete newMesh[m];
				newMesh[m] = nullptr;
			}
		}

		aiReleaseImport(scene);

	}
	else
	{
	}

}

void ModuleAssets::Render()
{
	for (int i = 0; i < meshList.size(); i++) {
		meshList[i]->DrawMesh();

	}
}


void MeshData::DrawMesh()
{

	glEnable(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	// ----------------------------------------------------------------------- Bind Buffers
	glBindBuffer(GL_ARRAY_BUFFER, id_vertex);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_index);

	glVertexPointer(3, GL_FLOAT, sizeof(float) * VERTEX_FEATURES, NULL);
	glNormalPointer(GL_FLOAT, sizeof(float) * VERTEX_FEATURES, NULL);

	glPushMatrix();

	// Draw
	glDrawElements(GL_TRIANGLES, num_index, GL_UNSIGNED_INT, NULL);

	glPopMatrix();

	// ----------------------------------------------------------------------- UnBind Buffers
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_TEXTURE_COORD_ARRAY);
}

void ModuleAssets::LoadMeshData(MeshData* mesh)
{
	glEnableClientState(GL_VERTEX_ARRAY);

	// ---------------------------------------------------- Buffer Creation
	glGenBuffers(1, (GLuint*)&(mesh->id_vertex));
	glGenBuffers(1, (GLuint*)&(mesh->id_index));

	// ----------------------------------------------------------------------- Bind Buffers
	glBindBuffer(GL_ARRAY_BUFFER, mesh->id_vertex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh->num_vertex * VERTEX_FEATURES, mesh->vertex, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->id_index);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * mesh->num_index, mesh->index, GL_STATIC_DRAW);

	glDisableClientState(GL_VERTEX_ARRAY);

	//Push Mesh to the List
	meshList.push_back(mesh);
}