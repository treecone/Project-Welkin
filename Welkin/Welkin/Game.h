#pragma once

#include "WkWindow.h"
#include "VulkanCore.h"
#include "FileManager.h"
#include <algorithm>
#include <chrono>
#include "Camera.h"
#include "ImGUI.h"
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
	ImGUI* imGui;
	FileManager* fileManager;
	//Input* input;
	Camera* mainCamera;

	float deltaTime;
	float FPS;
	unsigned int framesElapsed;
	float totalTimeSinceFPS;

	void Init();
	void CreateObject(string objName, string modelName, string materialFolderName, Transform transform = Transform(), bool sort = false);
	void SortObjectsByMaterial();
};