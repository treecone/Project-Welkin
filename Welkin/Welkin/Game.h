#pragma once

#include "WkWindow.h"
#include "VulkanCore.h"
#include "FileManager.h"

class Game
{
public:
	Game();
	~Game();
	void Update();

private:
	unsigned int WIDTH = 1024;
	unsigned int HEIGHT = 512;

	WkWindow* mainWindow;
	VulkanCore* vCore;
	FileManager* fileManager;

	void Init();
};

