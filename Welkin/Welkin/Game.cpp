#include "Game.h"

Game::Game()
{
	Init();
}

void Game::Init()
{
	Helper::Cout("Game Initalization", true);

	mainWindow = new WkWindow{ WIDTH, HEIGHT, "Main Welkin Window" };

	input = new Input(mainWindow);

	mainCamera = new Camera(3.0f, 1.0f, 45, (float)WIDTH / (float)HEIGHT, 0.1f, 1000, input);
	mainCamera->GetTransform()->SetPosition(0, 0, -10);

	vCore = new VulkanCore(mainWindow->GetWindow());

	fileManager = new FileManager(vCore);

	renderer = new Renderer(vCore, fileManager, mainCamera, &gameObjects);


	//imGui = new ImGUI(vCore, mainWindow, input);

	AssetCreation();

	Update();
}

Game::~Game()
{
	//Clean up all other vulkan resources before
	delete fileManager;
	delete mainCamera;
	delete renderer;
	delete vCore;
	//delete imGui;
	delete input;
	delete mainWindow;

	for (auto& gameObject : gameObjects)
	{
		delete gameObject;
	}
}

void Game::AssetCreation()
{
	Helper::Cout("Asset Creation", true);

	//TODO change the naming conventions of models and materials
	//CreateObject("Viking Cone", "Pyramid", "VikingRoom");

	Transform planeTransform(vec3(0, -3, 0), vec3(0, 0, 0), vec3(5, 5, 5));
	CreateObject("Main Plane", "SimplePlane", "Brick", planeTransform);

	Transform cubeTransform(vec3(3, 0, 0), vec3(0, 0, 0), vec3(2, 2, 2));
	CreateObject("Smooth Cube", "(HighPoly)SmoothCube", "Tiles115", cubeTransform);

	Transform vikingTransform(vec3(0, 1, 0), vec3(0, 0, 0), vec3(2, 2, 2));
	CreateObject("Smooth Cube", "VikingRoom", "Tiles115", vikingTransform);
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
	Helper::Cout("Game Loop", true);

	while (!mainWindow->shouldClose())
	{
		auto startTime = std::chrono::high_resolution_clock::now();
		//Helper::Cout("[" + std::to_string(framesElapsed) + "]");
		
		//MAIN LOOP
		{ 
			glfwPollEvents();
			input->Update();
			mainCamera->Update((float)deltaTime);

			if (input->KeyDown(GLFW_KEY_P))
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
				renderer->DrawFrame();
			}

			input->EndOfFrame();
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

void Game::SetScreenResolution(int width, int height)
{
	vCore->SetWindowSize(width, height);
	//TODO set this up
}