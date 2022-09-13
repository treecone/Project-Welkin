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


	//Camera
	const float nearPlane = 0.1f;
	const float farPlane = 1000;
	mainCamera = unique_ptr<Camera>(new Camera(3.0f, 1.0f, WIDTH / HEIGHT, nearPlane, farPlane));

	Helper::Cout("Game Loop", true);

	CreateObject("Viking Room", "VikingRoom", "VikingRoom");
}

Game::~Game()
{
	//Clean up all other vulkan resources before
	delete fileManager;
	delete vCore;
	delete mainWindow;

	for (int i = 0; i < gameObjects.size(); i++)
	{
		delete gameObjects[i];
	}
}

void Game::CreateObject(string objName, string modelName, string materialFolderName, bool sort)
{
	GameObject* newObj = new GameObject(objName, fileManager->FindMesh(modelName), fileManager->FindMaterial(materialFolderName));
	vector<GameObject*>::iterator location = upper_bound(gameObjects.begin(), gameObjects.end(), newObj);
	gameObjects.insert(location, newObj);

	Helper::Cout("[GameObject] Created " + newObj->name);

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

		for (GameObject* gObject : gameObjects)
		{
			gObject->Draw(vCore->GetLogicalDevice(), mainCamera.get());
		}

		#pragma region Getting FPS
			deltaSeconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - startTime).count();

			if (deltaSeconds - lastDeltaSeconds > 0)
			{
				lastDeltaSeconds = deltaSeconds;
				FPS = framesElapsed;
			}

			framesElapsed++;
		#pragma endregion

		mainCamera->Update(deltaSeconds);

	}

	vkDeviceWaitIdle(*vCore->GetLogicalDevice());
}

void Game::SortObjectsByMaterial()
{
	std::sort(gameObjects.begin(), gameObjects.end());
}
