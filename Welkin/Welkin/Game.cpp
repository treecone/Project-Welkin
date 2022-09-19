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
	vCore = new VulkanCore(mainWindow->GetWindow(), fileManager, &gameObjects);

	Helper::Cout("Game Initalization", true);

	//Camera
	const float nearPlane = 0.1f;
	const float farPlane = 1000;


	mainCamera = unique_ptr<Camera>(new Camera(3.0f, 1.0f, (float)WIDTH / (float)HEIGHT, nearPlane, farPlane));

	CreateObject("Viking Room", "VikingRoom", "VikingRoom");

	Helper::Cout("Game Loop", true);
	Update();
}

Game::~Game()
{
	//Clean up all other vulkan resources before
	delete fileManager;
	delete vCore;
	delete mainWindow;

	for (auto& gameObject : gameObjects)
	{
		delete gameObject;
	}
}

void Game::CreateObject(string objName, string modelName, string materialFolderName, bool sort)
{
	GameObject* newObj = new GameObject(objName, fileManager->FindMesh(modelName), fileManager->FindMaterial(materialFolderName));
	vector<GameObject*>::iterator location = upper_bound(gameObjects.begin(), gameObjects.end(), newObj);
	gameObjects.insert(location, newObj);

	//Technically shouldn't ever need to call this...
	if (sort)
		SortObjectsByMaterial();
}

void Game::Update()
{
	auto startTime = std::chrono::high_resolution_clock::now();
	unsigned int framesElapsed = 0;

	while (!mainWindow->shouldClose())
	{
		//Main loop
		glfwPollEvents();
		//vCore->DrawFrame();

		#pragma region Getting FPS
			deltaSeconds = (unsigned long)std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - startTime).count();

			if (deltaSeconds - lastDeltaSeconds > 0)
			{
				lastDeltaSeconds = deltaSeconds;
				FPS = framesElapsed;
			}

			framesElapsed++;
		#pragma endregion

		mainCamera->Update((float)deltaSeconds);

	}

	vkDeviceWaitIdle(*vCore->GetLogicalDevice());
}

void Game::SortObjectsByMaterial()
{
	std::sort(gameObjects.begin(), gameObjects.end());
}