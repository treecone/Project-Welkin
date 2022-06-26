#include "Game.h"

Game::Game()
{
	Init();
}

void Game::Init()
{
	mainWindow = new WkWindow{ WIDTH, HEIGHT, "Main Welkin Window" };

	//Init's the vulkan core
	vulkanCore = new VulkanCore(mainWindow->GetWindow());

}

Game::~Game()
{
	//Clean up all other vulkan resources before
	delete vulkanCore;
	delete mainWindow;
}

void Game::Update()
{
	while (!mainWindow->shouldClose())
	{
		glfwPollEvents();
	}
}
