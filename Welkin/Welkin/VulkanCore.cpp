#include "VulkanCore.h"

#define STB_IMAGE_IMPLEMENTATION //TODO fix this? 
#include <stb_image.h>

VulkanCore::VulkanCore(GLFWwindow* window, FileManager* fm)
{
	Helper::Cout("Vulkan Core", true);
	this->window = window;
	this->fm = fm;
	InitVulkan();
}

VulkanCore::~VulkanCore()
{
	CleanupSwapChain();

	#pragma region Descriptor Cleanup
		//VkDescriptorSets are auto cleaned up when pool is destroyed
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

		for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
		{
			vkDestroyBuffer(device, uniformBuffers[i], nullptr);
			vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
		}
	#pragma endregion

	#pragma region Buffer Cleanup
		vkDestroyBuffer(device, indexBuffer, nullptr);
		vkFreeMemory(device, indexBufferMemory, nullptr);

		vkDestroyBuffer(device, vertexBuffer, nullptr);
		vkFreeMemory(device, vertexBufferMemory, nullptr);
	#pragma endregion

	#pragma region Img Cleanup
		vkDestroyImageView(device, textureImageView, nullptr);
		vkDestroyImage(device, textureImage, nullptr);
		vkDestroySampler(device, textureSampler, nullptr);
		vkFreeMemory(device, textureImageMemory, nullptr);
	#pragma endregion


	//Graphics pipeline/stuff
	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);


	for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}

	//Setup
	vkDestroyCommandPool(device, commandPool, nullptr);

	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);

}


void VulkanCore::InitVulkan()
{
	//Setup
	CreateInstance();
	CreateSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();

	//Presentation
	CreateSwapchain();
	CreateImageViews();

	fm->Init(&this->device);

	Helper::Cout("Renderer", true);
	CreateRenderPass();
	CreateDescriptorSetLayout();
	CreateGraphicsPipeline();
	CreateFramebuffers();
	CreateCommandPool();

	CreateTextureImage();
	CreateTextureImageView();
	CreateTextureSampler();

	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSets();

	CreateCommandBuffers();
	CreateSyncObjects();

}

VkDevice* VulkanCore::GetLogicalDevice()
{
	if (this->device != NULL)
	{
		return &this->device;
	}
	else
		throw std::runtime_error("Trying to Access Logical Device before creation");
}


void VulkanCore::SetWindowSize(int width, int height)
{
	glfwSetWindowSize(window, width, height);
	frameBufferResized = true;
}

#pragma region Setup

	void VulkanCore::CreateInstance()
	{
		//Check for required extensions
		CheckAvaiableExtensions();

		//Check for required validation layers
		if (enableValidationLayers && !CheckValidationLayerSupport())
		{
			throw std::runtime_error("Validation layers requested, but not available!");
		}

		//Basic Information like application name and version
		#pragma region Application Info
			VkApplicationInfo appInfo{};
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pApplicationName = "Welkin Project";
			appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.pEngineName = "Welkin Engine";
			appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.apiVersion = VK_API_VERSION_1_0;
		#pragma endregion

		//Tells vulkan what global extensions and validation layers we want to use
		#pragma region VK Instance Create Info
			VkInstanceCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			createInfo.pApplicationInfo = &appInfo;
		#pragma endregion

		#pragma region VK Instance - GLFW Extensions
			unsigned int glfwExtensionCount = 0;
			const char** glfwExtensions;

			glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

			createInfo.enabledExtensionCount = glfwExtensionCount;
			createInfo.ppEnabledExtensionNames = glfwExtensions;
		#pragma endregion

		#pragma region VK Instance - Validation Layers
			if (enableValidationLayers) 
			{
				createInfo.enabledLayerCount = static_cast<uint32_t>(WantedValidationLayers.size());
				createInfo.ppEnabledLayerNames = WantedValidationLayers.data();
			}
			else 
			{
				createInfo.enabledLayerCount = 0;
			}

		#pragma endregion

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create instance!");
		}
	
		Helper::Cout("Instance Created!");
	}

	//Retrieves a list of supported extensions before creating the instance
	void VulkanCore::CheckAvaiableExtensions()
	{
		unsigned int extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		Helper::Cout("Avaiable Extensions Checked!");

		for (const auto& extension : extensions) 
		{
			//Helper::Cout("-" + (std::string)extension.extensionName);
		}
	}

	//Checks if all the requested validation layers (specified in the stuct) are avaiable
	bool VulkanCore::CheckValidationLayerSupport()
	{
		unsigned int validationLayerCount;
		vkEnumerateInstanceLayerProperties(&validationLayerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(validationLayerCount);
		vkEnumerateInstanceLayerProperties(&validationLayerCount, availableLayers.data());

		//Checking if we get all the layers we wanted
		#pragma region LayerCheck

			for (const char* layerName : WantedValidationLayers) {
				bool layerFound = false;

				for (const auto& layerProperties : availableLayers) 
				{
					if (strcmp(layerName, layerProperties.layerName) == 0) {
						layerFound = true;
						break;
					}
				}

				if (!layerFound)
				{
					Helper::Cout("[Warning] Did not find all requested Validation Layers!");
					return false;
				}
			}
			Helper::Cout("Found all requested Validation Layers!");
			return true;
		#pragma endregion
	}

	void VulkanCore::PickPhysicalDevice()
	{
		#pragma region GettingAllCards

			unsigned int physicalDeviceCount = 0;
			vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

			if (physicalDeviceCount == 0)
				throw std::runtime_error("failed to find GPUs with Vulkan support!");

			std::vector<VkPhysicalDevice> devices(physicalDeviceCount);
			vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, devices.data());
		#pragma endregion

		//foreach card....
		for (int i = 0; i < devices.size(); i++)
		{
			if (isDeviceSuitable(devices[i]))
			{
				physicalDevice = devices[i];
				Helper::Cout("Device chosen!");
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("failed to find suitable GPU!");
		}
	}

	bool VulkanCore::isDeviceSuitable(VkPhysicalDevice physicalDevice)
	{
		#pragma region Basic Properties and Features
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

			Helper::Cout("- Determining suitablity of card: " + (std::string)deviceProperties.deviceName);

			VkPhysicalDeviceFeatures deviceFeatures; //TODO call device features only oncec
			vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
		#pragma endregion

		QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

		bool extensionsSupported = CheckDeviceExtensionSupport(physicalDevice);

		//Check for the swap chain 
		bool swapchainAdequate = false;
		if (extensionsSupported)
		{
			SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(physicalDevice);
			swapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
		}

		return indices.isComplete() && extensionsSupported && swapchainAdequate && deviceFeatures .samplerAnisotropy;


	}

	bool VulkanCore::CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	VulkanCore::QueueFamilyIndices VulkanCore::FindQueueFamilies(VkPhysicalDevice physicalDevice)
	{
		QueueFamilyIndices indices;

		//Logic here to find the queue family indices and to populate the struct
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		//For each queue specified...
		for (int i = 0; i < queueFamilyCount; i++)
		{
			if (indices.isComplete())
			{
				break;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

			if (presentSupport) {
				indices.presentFamily = i;
			}

			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}
		}
		return indices;
	}

	void VulkanCore::CreateLogicalDevice()
	{
		QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

		#pragma region Queue Create Info Structs
			//Describes the amount of queues we want for each family
			std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
			std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
			float queuePriority = 1.0f;

			//For each queue...
			for (uint32_t queueFamily : uniqueQueueFamilies) 
			{
				VkDeviceQueueCreateInfo queueCreateInfo{};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = queueFamily;
				queueCreateInfo.queueCount = 1;
				queueCreateInfo.pQueuePriorities = &queuePriority;
				queueCreateInfos.push_back(queueCreateInfo);
			}
		#pragma endregion

		#pragma region PhysicalDeviceFeatures
			VkPhysicalDeviceFeatures deviceFeatures{};
			deviceFeatures.samplerAnisotropy = VK_TRUE;

		#pragma endregion

		#pragma region Device Create Info

			VkDeviceCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

			createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
			createInfo.pQueueCreateInfos = queueCreateInfos.data();

			createInfo.pEnabledFeatures = &deviceFeatures;

			createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
			createInfo.ppEnabledExtensionNames = deviceExtensions.data();

			if (enableValidationLayers) 
			{
				createInfo.enabledLayerCount = static_cast<uint32_t>(WantedValidationLayers.size());
				createInfo.ppEnabledLayerNames = WantedValidationLayers.data();
			}
			else 
			{
				createInfo.enabledLayerCount = 0;
			}

		#pragma endregion

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create logical device!");
		}
		Helper::Cout("Logical Device Created!");

		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentationQueue);
		Helper::Cout("Graphics and Presentation Queue Created!");

	}

	void VulkanCore::CreateSurface()
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface!");
		}
		Helper::Cout("Created Surface");
	}

#pragma endregion

#pragma region Presentation

	//Appearently the swapchain is on the GPU. ehh?
	VulkanCore::SwapchainSupportDetails VulkanCore::QuerySwapchainSupport(VkPhysicalDevice physicalDevice)
	{
		SwapchainSupportDetails details;

		//Basic surface capabilities (min/max # of images, min/max width height of images)
		#pragma region Basic Surface Capabilities
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);
		#pragma endregion

		//Pixel Format, Color Space
		#pragma region Supported Surface Formats
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
		}
	#pragma endregion

		//Avaiable Presentation Modes 
		#pragma region Present Modes
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
		}
	#pragma endregion

		return details;
	}

	void VulkanCore::CreateSwapchain()
	{
		SwapchainSupportDetails details = QuerySwapchainSupport(physicalDevice);

		//Gets color depth
		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(details.formats);

		//Gets present mode (aka Triple buffering)
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(details.presentModes);

		//Gets resolution of the images in the swap chain 
		VkExtent2D extent = ChooseSwapExtent(details.capabilities);

		#pragma region Get Number of images
			uint32_t imageCount = details.capabilities.minImageCount + 1;

			if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount)
			{
				imageCount = details.capabilities.maxImageCount;
			}
		#pragma endregion

		#pragma region Create Info
			VkSwapchainCreateInfoKHR createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfo.surface = surface;

			createInfo.minImageCount = imageCount;
			createInfo.imageFormat = surfaceFormat.format;
			createInfo.imageColorSpace = surfaceFormat.colorSpace;
			createInfo.imageExtent = extent;
			createInfo.imageArrayLayers = 1;
			//Change this to VK_IMAGE_USAGE_TRANSFER_DST_BIT if POST PROCESS
			createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			//What transforms should be applied (like 90 deg rotation)
			createInfo.preTransform = details.capabilities.currentTransform;

			//compositeAlpha field specifies if the alpha channel should be used for blending with other windows in the window system
			createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

			createInfo.presentMode = presentMode;
			//We dont care about the color of pixels are are obscured
			createInfo.clipped = VK_TRUE;

			//Put a old swap chain here if recreating the whole thing
			createInfo.oldSwapchain = VK_NULL_HANDLE;


		#pragma endregion

		#pragma region Handling SC images across multiple queue families
			QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);
			uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

			if (indices.graphicsFamily != indices.presentFamily)
			{
				createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				createInfo.queueFamilyIndexCount = 2;
				createInfo.pQueueFamilyIndices = queueFamilyIndices;
			}
			else
			{
				createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
				createInfo.queueFamilyIndexCount = 0; // Optional
				createInfo.pQueueFamilyIndices = nullptr; // Optional
			}
		#pragma endregion

		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create swap chain!");
		}

		Helper::Cout("Created Swap Chain!");

		#pragma region Retrieving the handles to the image views inside 
			vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
			swapChainImages.resize(imageCount);

			vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
		#pragma endregion

			swapChainImageFormat = surfaceFormat.format;
			swapChainExtent = extent;
	}

	#pragma region SwapChainDetails

		//Gets the color depth
		//Each VkSurfaceFormat contains a format (color channels) and colorspace (indicates if SRGB is supported)
		VkSurfaceFormatKHR VulkanCore::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
		{
			for (const auto& availableFormat : availableFormats)
			{
				if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				{
					return availableFormat;
				}
			}

			return availableFormats[0];
		}

		//(Immediate, FIFO, RELAXED FIFO, MAILBOX(Triple Buffering))
		VkPresentModeKHR VulkanCore::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
		{
			for (const auto& availablePresentMode : availablePresentModes)
			{
				if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					return availablePresentMode;
				}
			}

			return VK_PRESENT_MODE_FIFO_KHR;
		}

		//Resolution of the swap chain images
		VkExtent2D VulkanCore::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
		{
			//If the extent is set to unint32 max -> this means that the window manager allows us to differ
			if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
			{
				return capabilities.currentExtent;
			}
			else
			{
				int width, height;
				glfwGetFramebufferSize(window, &width, &height);

				VkExtent2D actualExtent =
				{
					static_cast<uint32_t>(width),
					static_cast<uint32_t>(height)
				};

				actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
				actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

				return actualExtent;
			}
		}

	#pragma endregion

	void VulkanCore::CreateImageViews()
	{
		swapChainImageViews.resize(swapChainImages.size());

		for (unsigned int i = 0; i < swapChainImages.size(); i++) 
		{
			swapChainImageViews[i] = CreateImageView(swapChainImages[i], swapChainImageFormat);
		}

		Helper::Cout("Created Image Views!");
	}

	/*
	That's all it takes to recreate the swap chain! However, 
	the disadvantage of this approach is that we need to stop 
	all rendering before creating the new swap chain. It is 
	possible to create a new swap chain while drawing commands
	on an image from the old swap chain are still in-flight. 
	You need to pass the previous swap chain to the oldSwapChain
	field in the VkSwapchainCreateInfoKHR struct and destroy the 
	old swap chain as soon as you've finished using it.
	*/
	void VulkanCore::CleanupSwapChain()
	{
		for (unsigned int i = 0; i < swapChainFrameBuffers.size(); i++) 
		{
			vkDestroyFramebuffer(device, swapChainFrameBuffers[i], nullptr);
		}

		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			vkDestroyImageView(device, swapChainImageViews[i], nullptr);
		}

		vkDestroySwapchainKHR(device, swapChain, nullptr);
	}

	void VulkanCore::RecreateSwapChain()
	{
		#pragma region Handling Minimization
		//Waits for the window to have a greater width/height then 0 to continue
			int width = 0, height = 0;
			glfwGetFramebufferSize(window, &width, &height);
			while (width == 0 || height == 0)
			{
				glfwGetFramebufferSize(window, &width, &height);
				glfwWaitEvents();
			}
		#pragma endregion

		Helper::Cout("[Recreating Swap Chain]");

		//Wait so we don't use resources that are currently being used
		vkDeviceWaitIdle(device);

		//Make sure the old resources are cleaned up
		CleanupSwapChain();

		//Recreate swapchain
		CreateSwapchain();

		//May potentially need to recreate the renderpass because u are moving the window from
		//a standard to high range dynamic monitor

		//CreateRenderpass();

		//Image views need to be recreated because they are based directly on the swap chain images
		CreateImageViews();
		//The framebuffers directly depend on the swap chain images, and thus must be recreated as well 
		CreateFramebuffers();
	}

#pragma endregion

#pragma region Graphics Pipelines/Render Passes/Pipeline

	/*
	Steps:
	- Wait for previous frame
	- Acquire an image from the swap chain
	- Record a command buffer which draws the scene onto that image
	- Submit the cmd buffer to a queue
	- Present the swap chain

	Semaphores - Handle operations in queues (cmd buffers, or functions), on the GPU
	- Begins as unsiginaled
	- Does not block exacution
	- When we submit a cmd, we can change and check for singletons

	Fences - Similar Purpose, but is used for ordering the exacution on the CPU
	- Also done during QueueSubmit, but uses vkWaitForFence to wait on CPU
	- Does block exacution
	- Starts Unsignaled
	- Dont block host unless necessary
	*/
	void VulkanCore::DrawFrame()
	{
		//Waits for signaled------------------------
		vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

		//Aquire Image from swap chain -------------------
		unsigned int imageIndex;
		VkResult swapChainResult = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		if (swapChainResult == VK_ERROR_OUT_OF_DATE_KHR)
		{
			//If the swap chain is not right, return to get out of the loop
			RecreateSwapChain();
			return;
		}
		else if (swapChainResult != VK_SUCCESS && swapChainResult != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to aquire swap chain image!");
		}

		//Reset fence only if we are submitting work 
		vkResetFences(device, 1, &inFlightFences[currentFrame]);

		//Record Command Buffer -----------------------
		//This makes sure that the cmd buffer is able to be recorded
		vkResetCommandBuffer(commandBuffers[currentFrame], 0);
		RecordCommandBuffer(commandBuffers[currentFrame], imageIndex);

		UpdateUniformBuffer(currentFrame);

		//Queue Submission -------------------------
		#pragma region Submit Info
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		//Wait in the pipeline color stage for the imageAvaiableSemaphore
		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame]};
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		//Actual cmd buffer we want to submit
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

		//What semaphores to signal once ^ are finished
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame]};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;
		#pragma endregion

		//Optional fence to be signaled after
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		//Presentation --------------------------------
		//Submit the result back to the swap chain to have it eventually show up on the screen.

		//Presentation is configured through this
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		//What semaphores to wait for
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		VkResult queuePresentResult = vkQueuePresentKHR(presentationQueue, &presentInfo);
		if (queuePresentResult == VK_ERROR_OUT_OF_DATE_KHR || queuePresentResult == VK_SUBOPTIMAL_KHR || frameBufferResized)
		{
			frameBufferResized = false;
			RecreateSwapChain();
		}
		else if (queuePresentResult != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image!");
		}
	}

	//Render pass -> Tells vulkan about the framebuffers attachments.
	// "The attachments refrenced by the pipeline stages and their usage"
	// Determines if we should clear new frame, and what to do with the data of the frames after being displyed
	//Specifies how many color and depth buffers there will be, how many samplers to use for each of them, and how their contents will be handled
	void VulkanCore::CreateRenderPass()
	{
		#pragma region Color and Formats, Clearing New Frames
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		//Determines what should we do with the data in the attachment before rendering and after rendering
		//Options LoadOP: LOAD OP LOAD - perserve contents, LOAD OP CLEAR - Clear the values to a constant, LOAD OP DONT CARE - We dont care about them
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		//Options for storeOP: STORE OP STORE - Rendered contents will be stored in memory and be read later, STORE OP DONT CARE - Dont
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		//Now for the same thing, but with stencils. CHANGE THIS IF USING STENCILS
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		//Images are going to need to be transitioned, so this in the initial format
		//For example, Color attachment, present format, transfer format
		//Before Render pass
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		//After Render pass
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		#pragma endregion

		//Refrences the past passes, including dependancies and such
		#pragma region Subpasses

	//Create more of these if u need more then just a color attachment
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		//-------------------------------------

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		#pragma endregion

		#pragma region Subpass Dependencies
		//Specify memory managment and execution dependencies between subpasses
		//Even if we only have one, the operations before and after this one subpass count 

		VkSubpassDependency dependency{};
		//SUBPASS_EXTERNAL = subpass before or after the render pass depending on if its src or dst 
		//dst must be > then src subpass (unless SUBPASS_EXTERNAL)
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;

		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;

		//Prevents the transition from happening until we actual want to start writing colors to it
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		#pragma endregion

		#pragma region RenderPassInfo
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;
		#pragma endregion

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create render pass!");
		}

		Helper::Cout("Created Render Pass!");
	}

	void VulkanCore::CreateGraphicsPipeline()
	{
		VkShaderModule* vertShaderModule = &fm->allShaders.find("(C)SimpleShaderVert.spv")->second;
		VkShaderModule* fragShaderModule = &fm->allShaders.find("(C)SimpleShaderFrag.spv")->second;

		#pragma region Shader Stage Creation (programmable functions)
		//Actually assign the shaders to specific pipeline stages
		#pragma region Vertex 
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

		//Specifies the shader module containing the code, and the function to invoke
		vertShaderStageInfo.module = *vertShaderModule;
		//ITS POSSIBLE TO HAVE MULTIPLE SHADERS IN A MODULE BC OF THE ENTRY POINT
		vertShaderStageInfo.pName = "main";
#pragma endregion

		#pragma region Fragment
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = *fragShaderModule;
		fragShaderStageInfo.pName = "main";
#pragma endregion

		#pragma endregion

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		//Fixed Functions --------------------------------------------

		//Format of the vertex data passed to the vertex shader
		#pragma region Vertex Input

		//How vertex data will be passed to the vertex shader
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};

		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();

		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		//Spacing between data and per-vertex/per-instance 9instancing)
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		#pragma endregion

		//(1) What kind of geometry will be drawn from the verties
		//(2) What type of primitive restart will be enabled (such as triangle list, struo, and line strip)
		#pragma region Input Assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;
		#pragma endregion

		//Could be defined in dynamic states
		#pragma region ViewPort and Scissors
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		/* DONE IN DYNAMIC STATES
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapChainExtent->width;
		viewport.height = (float)swapChainExtent->height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;
		*/
		#pragma endregion

		//Depth, linewidth, face culling, depthclamp cull, polygon mode
		#pragma region Rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		//Fragments that are beyond the near and far planes are clamped vs discarding them
		rasterizer.depthClampEnable = VK_FALSE;
		//Geo passed through the rasterizer stage
		rasterizer.rasterizerDiscardEnable = VK_FALSE;

		//Polygon (Fill, Line, Point)
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;

		//Cull Front-back, or both, or neither
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

#pragma endregion

		//disabled for now...
		#pragma region Multisampling
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional
#pragma endregion

		//need to implement...
		#pragma region Depth and Stencil Testing
	//VkPipelineDepthStencilStateCreateInfo stencilInfo{};
		#pragma endregion

		#pragma region Color Blending
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional
		#pragma endregion

		//States that can be changed during runtime
		#pragma region Dynamic States
		std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();
		#pragma endregion

		//Uniform values are variables that can be changed at drawing time. (etc: Transformation Matrix)
		//The uniform and push values refrenced by the shader that can be updated at draw time
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}

		#pragma region Creating The actual pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;

		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;

		pipelineInfo.layout = pipelineLayout;

		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;

		//Creating a new pipeline from a existing one - less expensive!
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional
		#pragma endregion

		//Could potentally create multuple graphis pipelines in one call. 
		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		Helper::Cout("Created Graphics Pipeline!");

	}

	//Render pass expects a framebuffers with the same format as teh swap chain images
	//The attachments that are specified during render pass creation are bound by wrapping them into framebuffers obects.
	//Framebuffers refrence all vkImageView ibhects that represent the attachments
	//Thus we need to create a framebuffer for all the images in the swap chain, using the one that corresponds to the retrieved image at drawing time
	void VulkanCore::CreateFramebuffers()
	{
		swapChainFrameBuffers.resize(swapChainImageViews.size());


		//Iterate through the image views and create framebuffers from them
		for (size_t i = 0; i < swapChainImageViews.size(); i++)
		{
			VkImageView attachments[] = { swapChainImageViews.at(i) };

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFrameBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create framebuffer!");
			}
		}

		Helper::Cout("Created Framebuffers");
	}

	//Command Pools are put onto indivisual queues
	void VulkanCore::CreateCommandPool()
	{
		VulkanCore::QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(physicalDevice);

		#pragma region Pool Info
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		//Change this if we are not recording a command buffer every frame
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		#pragma endregion

		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create command pool!");
		}

		Helper::Cout("Created Command Pool!");
	}

	void VulkanCore::CreateCommandBuffers()
	{
		commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		//Change this to secondary if this command buffer is only called from another one
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}

		Helper::Cout("Created CommandBuffer!");
	}

	void VulkanCore::CreateSyncObjects()
	{
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		//Starts Signaled
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create semaphores!");
			}
		}

		Helper::Cout("Created Sync Objects!");
	}

	//Writes commands we want to exacute into the command buffer
	//image index is the index of the currently swapchain image we want to write to
	void VulkanCore::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		#pragma region Command Buffer Basic Info
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//How to use the command buffer(etc. Secondary cmd buf, rerecorded right after exacuting once...)
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional, only relevent for secondary cmd buffers, what state to inherit
		#pragma endregion

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		#pragma region CmdBeginRenderPass
		VkRenderPassBeginInfo renderPassInfo{};
		//Render pass, and the attachments to bind
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFrameBuffers[imageIndex];

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;

		//Based on VK_ATTACHMENT_LOAD_OP_CLEAR in render pass
		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		//Change this if commands will be exacuted from secondary cmd buffers
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		#pragma endregion

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		#pragma region Buffers
			VkBuffer vertexBuffers[] = { vertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);
		#pragma endregion

		#pragma region Dynamic States
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChainExtent.width);
		viewport.height = static_cast<float>(swapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
		#pragma endregion

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}

		//Helper::Cout("Recorded Command Buffer!");


	}
#pragma endregion

//3 Steps for buffer - (Allocate Device Mem, Create Buffer, Bind them together)
#pragma region Buffers

	void VulkanCore::CreateVertexBuffer()
	{
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
		
		//Staging Buffer
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
		
		#pragma region Copy vertex data to staging buffer
			void* dataPointer;
			vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &dataPointer);
			memcpy(dataPointer, vertices.data(), (size_t)bufferSize);
			vkUnmapMemory(device, stagingBufferMemory);
		#pragma endregion

		//Because its DEVICE_LOCAL_BIT, cant map to it
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
		CopyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		//Clean it up
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	void VulkanCore::CreateIndexBuffer()
	{
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		#pragma region Copy vertex data to staging buffer
			void* data;
			vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, indices.data(), (size_t)bufferSize);
			vkUnmapMemory(device, stagingBufferMemory);
		#pragma endregion
		

		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

		CopyBuffer(stagingBuffer, indexBuffer, bufferSize);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	void VulkanCore::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) 
	{
		//Memory transfer operations are exacuted using cmd buffers
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		EndSingleTimeCommands(commandBuffer);
	}

	void VulkanCore::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		#pragma region Create Buffer

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create vertex buffer!");
		}
		#pragma endregion

		#pragma region Assign Memory
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}
		#pragma endregion

		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}

	uint32_t VulkanCore::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) 
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
			{
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	//Provides details about the descriptor binding used during pipeline creation
	//Describes the type of buffers that can be bound
	void VulkanCore::CreateDescriptorSetLayout()
	{
		#pragma region DescriptorSetLayoutBinding
				VkDescriptorSetLayoutBinding uboLayoutBinding{};
				uboLayoutBinding.binding = 0;
				uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				//Shader variable can represent an array of uniform buffer objects with count and binding 
				uboLayoutBinding.descriptorCount = 1;
				//Could also be SHADER_STAGE_ALL_GRAPHICS
				uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
				//Img sampling
				uboLayoutBinding.pImmutableSamplers = nullptr; // Optional
		#pragma endregion

		#pragma region SamplerLayoutBinding (for combined img sampler)
			VkDescriptorSetLayoutBinding samplerLayoutBinding{};
			samplerLayoutBinding.binding = 1;
			samplerLayoutBinding.descriptorCount = 1;
			samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			samplerLayoutBinding.pImmutableSamplers = nullptr;
			samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		#pragma endregion

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void VulkanCore::CreateUniformBuffers()
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

		for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
		}
	}

	void VulkanCore::UpdateUniformBuffer(uint32_t currentImage)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		#pragma region Actual Calculations
			UniformBufferObject ubo{};
			ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);

			//GLM flips the Y, so we need to correct it
			ubo.proj[1][1] *= -1;
		#pragma endregion

		void* dataPointer;
		vkMapMemory(device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &dataPointer);
		memcpy(dataPointer, &ubo, sizeof(ubo));
		vkUnmapMemory(device, uniformBuffersMemory[currentImage]);

	}

	void VulkanCore::CreateDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		//Max allocated, not avaiable, change it to "CREATE_FREE_DESCRIPTOR" if its changed after init
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void VulkanCore::CreateDescriptorSets()
	{		
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

		if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		//Populate the descriptors within
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
		{
			//Discriptors refer to the buffers, changes if its for imgs
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = textureImageView;
			imageInfo.sampler = textureSampler;

			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

			//For Uniform buffer
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			//Starting point of the ones to update
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			//How many discriptors to update
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;
			descriptorWrites[0].pImageInfo = nullptr; // Optional
			descriptorWrites[0].pTexelBufferView = nullptr; // Optional

			//For img
			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;

			//Can copy discriptors over with the latter
			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

#pragma endregion

#pragma region Image
	void VulkanCore::CreateTextureImage()
	{
		#pragma region Loading Img
			int texWidth, texHeight, texChannels;
			stbi_uc* pixels = stbi_load("Materials/Tiles115/Tiles115_1K_Color.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			VkDeviceSize imageSize = texWidth * texHeight * 4;

			if (!pixels) 
			{
				throw std::runtime_error("failed to load texture image!");
			}
		#pragma endregion

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(device, stagingBufferMemory);

		stbi_image_free(pixels);

		CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

		//Copy from staging buffer to texture img 
		//Transition the texture img to VK_MG_LAYOUT_TRANSGER_DST_OPTIMAL
		TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		CopyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	void VulkanCore::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(device, image, imageMemory, 0);
	}

	//TODO All of the helper functions that submit commands so far have been set up to execute synchronously by waiting for the queue to become idle. For practical applications it is recommended to combine these operations in a single command buffer and execute them asynchronously for higher throughput, especially the transitions and copy in the createTextureImage function. Try to experiment with this by creating a setupCommandBuffer that the helper functions record commands into, and add a flushSetupCommands to execute the commands that have been recorded so far. It's best to do this after the texture mapping works to check if the texture resources are still set up correctly.

	VkCommandBuffer VulkanCore::BeginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void VulkanCore::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}

	//Assuming here that the img has already been transition correctly
	void VulkanCore::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		#pragma region Image Mem Barrier
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			//Transfering what families to move from 
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			barrier.image = image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			#pragma region PipelineStages/AccessMasks
				VkPipelineStageFlags sourceStage;
				VkPipelineStageFlags destinationStage;

				if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
				{
					barrier.srcAccessMask = 0;
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

					sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
					destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				}
				else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
				{
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

					sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
					destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				}
				else 
				{
					throw std::invalid_argument("unsupported layout transition!");
				}
			#pragma endregion


			//https://vulkan-tutorial.com/Texture_mapping/Images
			vkCmdPipelineBarrier(
				commandBuffer,
				sourceStage, destinationStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);
		#pragma endregion

		EndSingleTimeCommands(commandBuffer);
	}

	void VulkanCore::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		#pragma region Region
			VkBufferImageCopy region{};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;

			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;

			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = {
				width,
				height,
				1
			};
		#pragma endregion

		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		EndSingleTimeCommands(commandBuffer);
	}

	void VulkanCore::CreateTextureImageView()
	{
		textureImageView = CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
	}

	VkImageView VulkanCore::CreateImageView(VkImage image, VkFormat format)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

		//Allows swizzling, for ex. mapping all channels to red
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create texture image view!");
		}

		return imageView;
	}

	void VulkanCore::CreateTextureSampler()
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		//NEAREST VS LINEAR
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);

		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		//Means that the texels are from (0, 1) instead of (0, width)
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		//Shadow map closer filtering
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		//Mip-mapping
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create texture sampler!");
		}
	}

#pragma endregion


