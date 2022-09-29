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

	mainCamera = new Camera(3.0f, 1.0f, 45, (float)WIDTH / (float)HEIGHT, nearPlane, farPlane);
	mainCamera->GetTransform()->SetPosition(0, 0, -10);

	Helper::Cout("Game Initalization", true);

	//Init's the vulkan core
	vCore = new VulkanCore(mainWindow->GetWindow(), fileManager, &gameObjects, mainCamera);

	//input = new Input();
	Input::GetInstance().InitInput(mainWindow);

	//imGui = new ImGUI(vCore, mainWindow, input);


	//TODO change the naming conventions of models and materials
	CreateObject("Viking Cone", "Pyramid", "VikingRoom");

	Transform planeTransform(vec3(0, 0, 0), vec3(0, 0, 0), vec3(5, 5, 5));
	CreateObject("Main Plane", "SimplePlane", "VikingRoom", planeTransform);

	Transform cubeTransform(vec3(3, 0, 0), vec3(0, 0, 0), vec3(2, 5, 1));
	CreateObject("Smooth Cube", "(HighPoly)SmoothCube", "VikingRoom", cubeTransform);

	Helper::Cout("Game Loop", true);
	Update();
}

Game::~Game()
{
	//Clean up all other vulkan resources before
	delete fileManager;
	delete mainCamera;
	delete vCore;
	//delete imGui;
	//delete input;
	delete mainWindow;

	for (auto& gameObject : gameObjects)
	{
		delete gameObject;
	}
}

void Game::CreateObject(string objName, string modelName, string materialFolderName, Transform transform, bool sort)
{
	GameObject* newObj = new GameObject(objName, fileManager->FindMesh(modelName), fileManager->FindMaterial(materialFolderName));
	newObj->GetTransform()->SetTransform(transform);
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
		//Helper::Cout("[" + std::to_string(framesElapsed) + "]");
		
		//MAIN LOOP
		{ 
			glfwPollEvents();
			Input::GetInstance().Update();
			mainCamera->Update((float)deltaTime);

			if (Input::GetInstance().KeyDown(GLFW_KEY_P))
			{
				const float rotateSpeed = 0.1f;
				gameObjects[0]->GetTransform()->Rotate(0, 0, rotateSpeed * (float)deltaTime);
			}

			for (auto& gameObj : gameObjects)
			{
				gameObj->GetTransform()->UpdateMatrices();
			}

			//imGui->Update((float)deltaTime);

			if (gameObjects.size() > 0)
			{
				if (framesElapsed == 0)
				{
					Helper::Cout("Begining to draw frame!");
				}
				vCore->DrawFrame();
			}

			Input::GetInstance().EndOfFrame();
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