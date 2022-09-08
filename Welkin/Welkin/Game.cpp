#include "Game.h"

Game::Game()
{
	Init();
}

void Game::Init()
{
	mainWindow = new WkWindow{ WIDTH, HEIGHT, "Main Welkin Window" };

	fileManager = new FileManager();

	//Init's the vulkan core
	vCore = new VulkanCore(mainWindow->GetWindow(), fileManager);
}

Game::~Game()
{
	//Clean up all other vulkan resources before
	delete fileManager;
	delete vCore;
	delete mainWindow;

	for (int i = 0; i < allGameobjects.size(); i++)
	{
		delete allGameobjects[i];
	}
}

void Game::CreateObjects()
{
	allGameobjects.push_back(new GameObject(fileManager->GetModel("viking_room"), fileManager->GetMaterial("Viking")));
}

void Game::Update()
{
	while (!mainWindow->shouldClose())
	{
		glfwPollEvents();
		vCore->DrawFrame();
	}

	vkDeviceWaitIdle(*vCore->GetLogicalDevice());

	vCore->currentFrame = (vCore->currentFrame + 1) & vCore->MAX_FRAMES_IN_FLIGHT;
}
