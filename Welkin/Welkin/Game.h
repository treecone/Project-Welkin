#pragma once

#include "WkWindow.h"
#include "VulkanCore.h"
#include "FileManager.h"
#include <algorithm>
#include <chrono>
#include "Camera.h"
#include "ImGUI.h"
#include "GameObject.h"
#include "Renderer.h"

class Game
{
public:
	Game();
	~Game();
	void Update();

	unsigned int WIDTH = 1024;
	unsigned int HEIGHT = 512;

private:

	WkWindow* mainWindow;
	VulkanCore* vCore;
	ImGUI* imGui;
	Renderer* renderer;
	FileManager* fileManager;
	Input* input;
	Camera* mainCamera;
	vector<GameObject*> gameObjects;

	float deltaTime;
	float FPS;
	unsigned int framesElapsed;
	float totalTimeSinceFPS;

	void Init();
	void AssetCreation();
	void CreateObject(string objName, string modelName, string materialFolderName, Transform transform = Transform(), bool sort = false);
	void SortObjectsByMaterial();
	void SetScreenResolution(int width, int height);
};