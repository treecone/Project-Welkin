#include "VulkanCore.h"

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

	//Sync Objects 
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
	{
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}

	//Renderer
	vkDestroyCommandPool(device, transferCommandPool, nullptr);
	vkDestroyCommandPool(device, graphicsCommandPool, nullptr);


	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);

	//Setup
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
	//CreateMemoryAllocator();

	//Presentation
	CreateSwapchain();
	CreateImageViews();

	//FileManager
	fm->Init(&this->device);

	//Rendering Stuff
	Helper::Cout("Pipeline and Passes", true);
	CreateRenderPass();
	CreateGraphicsPipeline();
	CreateFrameBuffers();
	CreateCommandPools();
	CreateCommandBuffers(graphicsCommandPool);
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
	framebufferResized = true;
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

		Helper::Cout("Avaiable Extensions Checked");
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
					Helper::Warning("Did not find all requested Validation Layers!");
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

		std::multimap<int, VkPhysicalDevice> candidates;

		//foreach card....
		for (int i = 0; i < devices.size(); i++)
		{
			int score = RateDeviceSuitability(devices[i]);
			candidates.insert(std::make_pair(score, devices[i]));
		}

		// Check if the best candidate is suitable at all
		if (candidates.rbegin()->first >= 1000) 
		{
			physicalDevice = candidates.rbegin()->second;

			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
			Helper::Cout("Using Card" + (string)deviceProperties.deviceName);
		}
		else 
		{
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}

	int VulkanCore::RateDeviceSuitability(VkPhysicalDevice physicalDevice)
	{
		int score = 0;

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

		if (indices.isComplete() && extensionsSupported && swapchainAdequate && deviceFeatures.samplerAnisotropy)
		{
			score += 1000;
		}

		if (indices.transferFamily != indices.graphicsFamily)
		{
			score++;
		}

		return score;
	}

	bool VulkanCore::CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) 
		{
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
			else if(queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				indices.transferFamily = i;
			}
		}

		if (!indices.transferFamily.has_value())
		{
			indices.transferFamily = indices.graphicsFamily;
		}

		return indices;
	}

	//TODO Create a second cmd pool for transfer queue, change to sharing mode concurrent, submit any transfer cmds like vkCmdCopyBuffer to the transfer queue instead

	void VulkanCore::CreateLogicalDevice()
	{
		QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

		#pragma region Queue Create Info Structs
			//Describes the amount of queues we want for each family
			std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
			std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value(), indices.transferFamily.value() };
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
		vkGetDeviceQueue(device, indices.transferFamily.value(), 0, &transferQueue);
		Helper::Cout("Graphics, transfer, and Presentation Queue Created!");

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

		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

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

		for (size_t i = 0; i < swapChainImages.size(); i++) 
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create image views!");
			}
		}

		Helper::Cout("Created Image Views!");
	}

	void VulkanCore::CleanupSwapChain()
	{
		
		for (unsigned int i = 0; i < swapChainFramebuffers.size(); i++) 
		{
			vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
		}
		
		for (size_t i = 0; i < swapChainImageViews.size(); i++) 
		{
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
		//CreateDepthResources();
		//The framebuffers directly depend on the swap chain images, and thus must be recreated as well 
		CreateFrameBuffers();
	}

#pragma endregion

#pragma region Pipeline/Passes

	void VulkanCore::CreateGraphicsPipeline()
	{
		VkShaderModule* vertShaderModule = fm->FindShaderModule("(C)SimpleShaderVert.spv");
		VkShaderModule* fragShaderModule = fm->FindShaderModule("(C)SimpleShaderFrag.spv");

		#pragma region Shader Stage Creation

			VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
			vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module = *vertShaderModule;
			vertShaderStageInfo.pName = "main";
			vertShaderStageInfo.pSpecializationInfo = nullptr; //Optional

			VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
			fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = *fragShaderModule;
			fragShaderStageInfo.pName = "main";

			VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		#pragma endregion

		#pragma region Vertex Shader Info

			//Discribes the format of the vertex data that will be passed to the vertex shader
			VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.vertexBindingDescriptionCount = 0;
			vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
			vertexInputInfo.vertexAttributeDescriptionCount = 0;
			vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional
		#pragma endregion

		#pragma region Input Assembly

			//What type of geometry will be drawn, and if primitive restart should be enabled
			VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssembly.primitiveRestartEnable = VK_FALSE;
		#pragma endregion

		#pragma region Viewport and Scissors

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

		#pragma region Rasterizer

			//Depth, linewidth, face culling, depthclamp cull, polygon mode
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

		#pragma region Multisampling

			//TODO implement
			VkPipelineMultisampleStateCreateInfo multisampling{};
			multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisampling.minSampleShading = 1.0f; // Optional
			multisampling.pSampleMask = nullptr; // Optional
			multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
			multisampling.alphaToOneEnable = VK_FALSE; // Optional
		#pragma endregion

		#pragma region Depth and Stencil

			//TODO implement
			VkPipelineDepthStencilStateCreateInfo depthInfo{};
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

		#pragma region DynamicStates
			//States that can be changed in runtime in the pipeline
			std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

			VkPipelineDynamicStateCreateInfo dynamicState{};
			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
			dynamicState.pDynamicStates = dynamicStates.data();
		#pragma endregion

		#pragma region Pipeline Layout

			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = 0; // Optional
			pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
			pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
			pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

			if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create pipeline layout!");
			}

		#pragma endregion

		#pragma region Creating the Pipeline

			VkGraphicsPipelineCreateInfo pipelineInfo{};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfo.stageCount = 2;
			pipelineInfo.pStages = shaderStages;

			//Use all the data above 
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

			if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) 
			{
				throw std::runtime_error("failed to create graphics pipeline!");
			}

		#pragma endregion

	}

	//Tells vulkan about the framebuffer attachments (aka color and depth buffers)
	//Includes what to do with data before and after rendering
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

			//Create more of these if u need more then just a color attachment in this renderpass
			VkAttachmentReference colorAttachmentRef{};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		#pragma endregion

		#pragma region Subpasses

			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;
			subpass.pDepthStencilAttachment = nullptr; //TODO add
		#pragma endregion

		#pragma region Subpass Dependencies
			//Making it so the render pass waits at the VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT stage 
			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0; //refers to first subpass

			//specify the operations to wait on and the stages in which these operations occur.
			//These settings will prevent the transition from happening until it's actually necessary (and allowed): when we want to start writing colors to it.
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		#pragma endregion


		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create render pass!");
		}

	}

	//Wraps the swapChainImageViews (aka render targets) in a framebuffer
	void VulkanCore::CreateFrameBuffers()
	{
		swapChainFramebuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); i++) 
		{
			VkImageView attachments[] = {swapChainImageViews[i]};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) 
			{
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	//Create one per thread
	void VulkanCore::CreateCommandPools()
	{
		QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(physicalDevice);

		//Graphics Cmd Pool
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		//Rerecorded often (but at the same time) or not (which is what is here)
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(device, &poolInfo, nullptr, &graphicsCommandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create graphics command pool!");
		}

		Helper::Cout("Created [Graphics] Cmd Pool");

		if (graphicsCommandPool != transferCommandPool)
		{
			//Transfer Cmd Pool
			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			poolInfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value();

			if (vkCreateCommandPool(device, &poolInfo, nullptr, &transferCommandPool) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create transfer command pool!");
			}
		}

		Helper::Cout("Created [Transfer] Cmd Pool");
	}

	void VulkanCore::CreateCommandBuffers(VkCommandPool pool)
	{
		mainCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = pool;
		//Specifies if the allocated command buffers are primary or secondary command buffers
		//aka Submitted to queue, or called from other cmd buffer
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)mainCommandBuffers.size();

		if (vkAllocateCommandBuffers(device, &allocInfo, mainCommandBuffers.data()) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}

		Helper::Cout("Created Command Buffers");

	}

	void VulkanCore::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		#pragma region Begin Cmd Buffer

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			//How to use the cmd buffer (aka rerecoreded after exacution, secondary cmd buffer, resubmitted while pending execution)
			beginInfo.flags = 0; // Optional
			beginInfo.pInheritanceInfo = nullptr; // Optional

			if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) 
			{
				throw std::runtime_error("failed to begin recording command buffer!");
			}
		#pragma endregion

		#pragma region Begin Render-Pass
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = swapChainExtent;

			//What to reset the color with
			VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			//Either Inline (no secondary cmd buffers) or subpass (cmds will be executed from a secondary cmd buffer)
			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		#pragma endregion

		#pragma region Dynamic States Setting
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

		//TODO add vertices here
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to record command buffer!");
		}

	}

#pragma endregion

#pragma region DrawFrame and Sync Objects

	/*
	Wait for the previous frame to finish
	Acquire an image from the swap chain
	Record a command buffer which draws the scene onto that image
	Submit the recorded command buffer
	Present the swap chain image
	*/

	void VulkanCore::DrawFrame()
	{
		//Wait until the previous frame has finished, aka waits for signaled
		vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

		//Aquire img from swap chain to draw to
		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) 
		{
			RecreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		//Sets fence(s) to unsignaled state
		vkResetFences(device, 1, &inFlightFences[currentFrame]);

		//Reset and record cmd buffer
		vkResetCommandBuffer(mainCommandBuffers[currentFrame], 0);
		RecordCommandBuffer(mainCommandBuffers[currentFrame], imageIndex);

		#pragma region Submit Cmd Buffer

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			//Use the img available semaphore to wait for images in the Ouput Bit Color stage
			VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame]};
			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;
			
			//What cmd buffer to submit
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &mainCommandBuffers[currentFrame];

			//What sempaphores to singal once finshed
			VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame]};
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;

			//inFlightFence is signaled after cmd buffer finishes exacution
			if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to submit draw command buffer!");
			}
		#pragma endregion	

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(presentationQueue, &presentInfo);


		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
		{
			framebufferResized = false;
			RecreateSwapChain();
		}
		else if (result != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to present swap chain image!");
		}

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
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
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
		{
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) 
			{

				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}

		Helper::Cout("Created Sync Objects");
	}

#pragma endregion
