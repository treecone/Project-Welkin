#pragma once
#include <vulkan/vulkan.h>
#include <vector>

#include "FileManager.h"
#include "Helper.h"
#include "VulkanCore.h"
#include "UniformBufferObject.h"
#include "StorageBufferObject.h"
#include "GameObject.h"

class Renderer
{
public:
	Renderer(VulkanCore* vCore, FileManager* fm, Camera* mainCamera, vector<GameObject*>* gameObjects);
	~Renderer();

	//Getters
	VkRenderPass* GetRenderPass() { return &this->renderPass; };

	//Drawing
	void DrawFrame();
	unsigned short currentFrame = 0;

private:

	FileManager* fm;
	VulkanCore* vCore;
	VkDevice* device;
	Camera* mainCamera;
	vector<GameObject*>* gameObjects;

#pragma region Pipeline/Passes

	void CreateGraphicsPipeline();
	void CreateRenderPass();

	VkRenderPass renderPass;
	VkPipeline graphicsPipeline;
	VkPipelineLayout pipelineLayout;

	//Commands ---------------

	void CreateCommandBuffers(VkCommandPool pool);
	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	std::vector<VkCommandBuffer> mainCommandBuffers;

	int bindTexturePipeline = 0;


#pragma endregion

#pragma region DrawFrame and Sync Objects
	void CreateSyncObjects();
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
#pragma endregion

#pragma region Buffers
	vector<UniformBufferObject*> allUniformBufferObjects;
	vector<StorageBufferObject*> allStorageBufferObjects;
#pragma endregion

};

