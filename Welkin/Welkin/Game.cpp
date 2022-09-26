#include "Game.h"

Game::Game()
{
	Init();
}

void Game::Init()
{
	mainWindow = new WkWindow{ WIDTH, HEIGHT, "Main Welkin Window" };

	fileManager = new FileManager();

	//Camera
	const float nearPlane = 0.1f;
	const float farPlane = 1000;

	mainCamera = new Camera(3.0f, 1.0f, (float)WIDTH / (float)HEIGHT, nearPlane, farPlane);
	mainCamera->GetTransform()->SetPosition(0, 0, -10);

	Helper::Cout("Game Initalization", true);

	//TODO change the naming conventions of models and materials
	CreateObject("Viking Room", "VikingRoom", "VikingRoom");

	//Init's the vulkan core
	vCore = new VulkanCore(mainWindow->GetWindow(), fileManager, &gameObjects, mainCamera);

	Helper::Cout("Game Loop", true);
	Update();
}

Game::~Game()
{
	//Clean up all other vulkan resources before
	delete fileManager;
	delete mainCamera;
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

	while (!mainWindow->shouldClose())
	{
		auto startTime = std::chrono::high_resolution_clock::now();
		
		//MAIN LOOP
		{ 
			glfwPollEvents();

			mainCamera->Update((float)deltaTime);

			const int rotateSpeed = 2;
			gameObjects[0]->GetTransform()->Rotate(0, 0, rotateSpeed * (float)deltaTime);

			//Rotate Objs
			for (auto& gameObj : gameObjects)
			{
				gameObj->GetTransform()->UpdateMatrices();
			}

			vCore->DrawFrame();
		}
		

		auto stopTime = std::chrono::high_resolution_clock::now();
		using ms = std::chrono::duration<float, std::milli>;
		deltaTime = std::chrono::duration_cast<ms>(stopTime - startTime).count();
		framesElapsed++;
		totalTimeSinceFPS += deltaTime;

		if (framesElapsed % 300)
		{
			//Calculate FPS every 300 frames
			FPS = (300.0f / totalTimeSinceFPS);
			totalTimeSinceFPS = 0;
		}
	}

	vkDeviceWaitIdle(*vCore->GetLogicalDevice());
}

void Game::SortObjectsByMaterial()
{
	std::sort(gameObjects.begin(), gameObjects.end());
}