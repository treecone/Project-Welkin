#include "VulkanCore.h"
#include <map>
#include <algorithm>

VulkanCore::VulkanCore(GLFWwindow* window)
{
	Helper::Cout("Vulkan Core", true);
	this->window = window;
	InitVulkan();
}

VulkanCore::~VulkanCore()
{
	CleanupSwapChain();

	//CommandPools
	vkDestroyCommandPool(device, transferCommandPool, nullptr);
	vkDestroyCommandPool(device, graphicsCommandPool, nullptr);

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

	//Presentation
	CreateSwapchain();
	CreateImageViews();

	CreateCommandPools();
}

VkDevice* VulkanCore::GetLogicalDevice()
{
	if (this->device != NULL)
	{
		return &this->device;
	}
	else
	{
		throw std::runtime_error("Trying to Access Logical Device before creation");
	}
}

VkPhysicalDevice* VulkanCore::GetPhysicalDevice()
{
	if (this->device != NULL)
	{
		return &this->physicalDevice;
	}
	else
	{
		throw std::runtime_error("Trying to Access Physical Device before creation");
	}
}

VkQueue* VulkanCore::GetQueue(int type)
{
	switch (type)
	{
	default:
	case(0):
		return &this->graphicsQueue;
		break;
	case(1):
		return &this->presentationQueue;
		break;
	case(2):
		return &this->transferQueue;
	}
}

VkCommandPool* VulkanCore::GetCommandPool(int type)
{
	switch (type)
	{
	default:
	case(0):
		return &this->graphicsCommandPool;
		break;
	case(1):
		return &this->transferCommandPool;
		break;
	}
}

VkPhysicalDeviceProperties VulkanCore::GetPhysicalDeviceProperties()
{
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);
	return properties;
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
			appInfo.apiVersion = VK_API_VERSION_1_1;
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
			Helper::Cout("Using Card" + (std::string)deviceProperties.deviceName);

			if (deviceProperties.limits.maxPushConstantsSize < sizeof(vHelper::PushConstants))
			{
				throw std::runtime_error("Allowed push constant size is too small");
			}
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

			//Indexing - aka bindless descriptors
			VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures{};
			indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
			indexingFeatures.pNext = nullptr;

			VkPhysicalDeviceFeatures2 deviceFeatures2{};
			deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			deviceFeatures2.pNext = &indexingFeatures;
			vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures2);

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

		if (indices.isComplete() && extensionsSupported && swapchainAdequate && deviceFeatures.samplerAnisotropy && indexingFeatures.descriptorBindingPartiallyBound && indexingFeatures.runtimeDescriptorArray)
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
		else
		{
			//Helper::Cout("Found seperate transfer family");
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

		#pragma region Indexing - aka Bindless Descriptors
			VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures{};
			indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
			indexingFeatures.pNext = nullptr;
			indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
			indexingFeatures.runtimeDescriptorArray = VK_TRUE;
		#pragma endregion


		#pragma region Device Create Info

			VkDeviceCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

			//Indexing 
			createInfo.pNext = &indexingFeatures;

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

		for (uint32_t i = 0; i < swapChainImages.size(); i++) 
		{
			swapChainImageViews[i] = CreateImageView(swapChainImages[i], swapChainImageFormat);
		}

		Helper::Cout("Created Image Views!");
	}

	//Wraps the swapChainImageViews (aka render targets) in a framebuffer
	void VulkanCore::CreateFrameBuffers(VkRenderPass* renderPass)
	{
		if (renderPass == nullptr)
		{
			throw std::exception("No current render pass for creating framebuffers!");
		}

		this->currentRenderPass = renderPass;
		swapChainFramebuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); i++)
		{
			VkImageView attachments[] = { swapChainImageViews[i] };

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = *renderPass;
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

		if (queueFamilyIndices.graphicsFamily.value() != queueFamilyIndices.transferFamily.value())
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

			Helper::Cout("Created [Transfer] Cmd Pool");
		}

	}


#pragma region Buffers

	void VulkanCore::CreateBuffer(const VkDeviceSize size, const VkBufferUsageFlags usage, const VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;

		#pragma region Sharing mode

			const QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);
			const uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.transferFamily.value() };

			if (indices.graphicsFamily != indices.transferFamily)
			{
				bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
				bufferInfo.queueFamilyIndexCount = 2;
				bufferInfo.pQueueFamilyIndices = queueFamilyIndices;
			}
			else
			{
				bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			}
		#pragma endregion

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create vertex buffer!");
		}

		//Now bind memory
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		//Find right type of memory
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		//VK_MEMORY_PROPERTY_HOST_COHERENT_BIT = ensures that the mapped memory always matches the contents of the allocated memory
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}
		//Associate this memory with the buffer
		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}

	void VulkanCore::CopyBuffer(const VkBuffer srcBuffer, const VkBuffer dstBuffer, const VkDeviceSize size)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands(transferCommandPool);

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		EndSingleTimeCommands(commandBuffer, transferQueue, transferCommandPool);
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

		//TODO check if this format is supported
		imageInfo.format = format;
		//If I want to directly access texels in the memory of the img, then switch this
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

		#pragma region Sharing mode

			const QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);
			const uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.transferFamily.value() };

			if (indices.graphicsFamily != indices.transferFamily)
			{
				imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
				imageInfo.queueFamilyIndexCount = 2;
				imageInfo.pQueueFamilyIndices = queueFamilyIndices;
			}
			else
			{
				imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			}
		#pragma endregion

		//Mulitsampling
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0; // Optional

		if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(device, image, imageMemory, 0);
	}

	uint32_t VulkanCore::FindMemoryType(const uint32_t type_filter, const VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((type_filter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	VkCommandBuffer VulkanCore::BeginSingleTimeCommands(VkCommandPool pool)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = pool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void VulkanCore::EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(queue);

		vkFreeCommandBuffers(device, pool, 1, &commandBuffer);
	}

	void VulkanCore::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands(graphicsCommandPool);
		
		#pragma region Barrier
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;

			//TODO try the other mode of sharing and change this
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			barrier.image = image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

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

			vkCmdPipelineBarrier(
				commandBuffer,
				sourceStage, destinationStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);
		#pragma endregion

		

		EndSingleTimeCommands(commandBuffer, graphicsQueue, graphicsCommandPool);
	}

	void VulkanCore::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands(transferCommandPool);

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {width, height, 1};

		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		EndSingleTimeCommands(commandBuffer, transferQueue, transferCommandPool);
	}

	VkImageView VulkanCore::CreateImageView(VkImage image, VkFormat format)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
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

	void VulkanCore::CreateDescriptorsForTextures()
	{

	}

#pragma endregion
