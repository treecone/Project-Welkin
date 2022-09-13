#pragma once

#include "WkWindow.h"
#include "VulkanCore.h"
#include "FileManager.h"
#include <algorithm>
#include <chrono>
#include "Camera.h"
#include "GameObject.h"

class Game
{
public:
	Game();
	~Game();
	void Update();

	vector<GameObject*> gameObjects;

	unsigned int WIDTH = 1024;
	unsigned int HEIGHT = 512;

private:

	WkWindow* mainWindow;
	VulkanCore* vCore;
	FileManager* fileManager;

	unique_ptr<Camera> mainCamera;

	unsigned long deltaSeconds;
	unsigned long lastDeltaSeconds;
	unsigned int FPS;

	void Init();
	void CreateObject(string objName, string modelName, string materialFolderName, bool sort = false);
	void SortObjectsByMaterial();
};