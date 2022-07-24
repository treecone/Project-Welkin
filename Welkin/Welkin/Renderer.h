#pragma once
#include "VulkanCore.h"
#include "FileManager.h"
#include <GLFW/glfw3.h>
#include "Helper.h"
class Renderer
{
public:
	Renderer(VulkanCore* VulkanCore, FileManager* fm);
	void DrawFrame();
	~Renderer();
private:
	//Imported
	VulkanCore* vCore;
	FileManager* fm;
	VkDevice* device;
	VkExtent2D* swapChainExtent;
	VkSwapchainKHR* swapchain;
	std::vector<VkImageView>* swapChainImageViews;
	VkQueue* graphicsQueue;
	VkQueue* presentationQueue;

	//Local
	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	std::vector<VkFramebuffer> swapChainFrameBuffers;

	//Command Stuff
	VkCommandPool commandPool;
	VkCommandBuffer commandBuffer;

	//Sync Objects
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	VkFence inFlightFence;

	void CreateRenderPass();
	void CreateGraphicsPipeline();
	void CreateFramebuffers();
	void CreateCommandPool();
	void CreateCommandBuffer();
	void CreateSyncObjects();
	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void Init();
};

