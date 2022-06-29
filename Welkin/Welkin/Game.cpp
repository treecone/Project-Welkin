#include "Game.h"

Game::Game()
{
	Init();
}

void Game::Init()
{
	mainWindow = new WkWindow{ WIDTH, HEIGHT, "Main Welkin Window" };
	fileManager = new FileManager();
	vulkanCore = new VulkanCore(mainWindow->GetWindow(), fileManager);
	renderer = new Renderer();

	Helper::Cout("Update", true);
	Update();

}

Game::~Game()
{
	//Clean up all other vulkan resources before
	delete vulkanCore;
	delete mainWindow;
	delete fileManager;
	delete renderer;
}

void Game::Update()
{
	while (!mainWindow->shouldClose())
	{
		glfwPollEvents();
	}
}
