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
	createSyncObjects();
	Update();

}

//Render --------------------------------

void Game::DrawFrame()
{
	/* RENDERING A FRAME =
	
	Wait for the previous frame to finish
	Acquire an image from the swap chain
	Record a command buffer which draws the scene onto that image
	Submit the recorded command buffer
	Present the swap chain image
	*/

	vkWaitForFences(vulkanCore->device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);

	vkResetFences(vulkanCore->device, 1, &inFlightFence);

	//Aquire img from swap chain
	uint32_t imageIndex;
	vkAcquireNextImageKHR(vulkanCore->device, vulkanCore->swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	//Makes sure that the command buffer is able to be recorded
	vkResetCommandBuffer(vulkanCore->commandBuffer, 0);

	vulkanCore->RecordCommandBuffer(vulkanCore->commandBuffer, imageIndex);

	//Now that its recorded, submit it
	#pragma region Queue submit data
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		//Specify what stage to wait at (this way we are still exacuting the vertex shader)
		submitInfo.pWaitDstStageMask = waitStages;

		//What command buffers we want 
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &vulkanCore->commandBuffer;

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;
	#pragma endregion

	if (vkQueueSubmit(vulkanCore->graphicsQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	//Now submit the result back to the swap chain 
	//GO OVER THIS AGAIN
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { vulkanCore->swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	presentInfo.pResults = nullptr; // Optional

	vkQueuePresentKHR(vulkanCore->presentationQueue, &presentInfo);

}

void Game::createSyncObjects()
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	//Starts signaled
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateSemaphore(vulkanCore->device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS || vkCreateSemaphore(vulkanCore->device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS || vkCreateFence(vulkanCore->device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create semaphores or fences!");
	}
}

Game::~Game()
{
	//Clean up all other vulkan resources before
	delete vulkanCore;
	delete mainWindow;
	delete fileManager;
	delete renderer;

	vkDestroySemaphore(vulkanCore->device, imageAvailableSemaphore, nullptr);
	vkDestroySemaphore(vulkanCore->device, renderFinishedSemaphore, nullptr);
	vkDestroyFence(vulkanCore->device, inFlightFence, nullptr);
}

void Game::Update()
{
	while (!mainWindow->shouldClose())
	{
		glfwPollEvents();
		DrawFrame();
	}

	vkDeviceWaitIdle(vulkanCore->device);
}
