#pragma once

#include "WkWindow.h"
#include "VulkanCore.h"
#include "FileManager.h"

#include "GameObject.h"

class Game
{
public:
	Game();
	~Game();
	void Update();

	vector<GameObject*> allGameobjects;

private:
	unsigned int WIDTH = 1024;
	unsigned int HEIGHT = 512;

	WkWindow* mainWindow;
	VulkanCore* vCore;
	FileManager* fileManager;

	void Init();
	void CreateObjects();
};

