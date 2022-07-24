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

	fileManager = new FileManager(vulkanCore->GetLogicalDevice());

	renderer = new Renderer(vulkanCore, fileManager);

}

Game::~Game()
{
	//Clean up all other vulkan resources before
	delete renderer;
	delete fileManager;
	delete vulkanCore;
	delete mainWindow;
}

void Game::Update()
{
	while (!mainWindow->shouldClose())
	{
		glfwPollEvents();
		renderer->DrawFrame();
	}

	vkDeviceWaitIdle(*vulkanCore->GetLogicalDevice());
}
