#include "VulkanCore.h"

VulkanCore::VulkanCore(GLFWwindow* window)
{
	Helper::Cout("Vulkan Core", true);
	this->window = window;
	InitVulkan();
}

VulkanCore::~VulkanCore()
{
	for (auto imageView : swapChainImageViews) 
	{
		vkDestroyImageView(device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(device, swapChain, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroyInstance(instance, nullptr);
}

VkPhysicalDevice* VulkanCore::GetPhysicalDevice()
{
	if (this->physicalDevice != NULL)
	{
		return &this->physicalDevice;
	}
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

VkExtent2D* VulkanCore::GetSwapChainExtent()
{
	return &this->swapChainExtent;
}

VkFormat* VulkanCore::GetSwapChainImageFormat()
{
	if (this->swapChainImageFormat != NULL)
		return &this->swapChainImageFormat;
	else
		throw std::runtime_error("Trying to Access Logical Device before creation");
}

VkSwapchainKHR* VulkanCore::GetSwapChain()
{
	return &this->swapChain;
}

VkQueue* VulkanCore::GetQueue(char queueNum)
{
	switch (queueNum)
	{
	default:
	case (0):
		return &this->graphicsQueue;
		break;

	case(1):
		return &this->presentationQueue;
		break;
	}
}

std::vector<VkImage>* VulkanCore::GetSwapChainImages()
{
	if (swapChainImages.size() > 0)
		return &this->swapChainImages;

	return nullptr;
}

std::vector<VkImageView>* VulkanCore::GetSwapChainImageViews()
{
	if (this->swapChainImageViews.size() > 0)
		return &this->swapChainImageViews;

	return nullptr;
}



void VulkanCore::InitVulkan()
{
	CreateInstance();
	CreateSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateSwapchain();
	CreateImageViews();
}

//----------------------------Vulkan Instance Creation -----------------------------

//Creates the vulkan instance
void VulkanCore::CreateInstance()
{
	//Check for required extensions
	CheckAvaiableExtensions();

	//Check for required validation layers
	if (enableValidationLayers && !CheckValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
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
	else
	{
		Helper::Cout("Instance Created!");
	}
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



//------------------------------Physical Device---------------------------------------------

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

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
	#pragma endregion

	//Can check for things such as if a card is a VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU 
	/*
		if (!deviceFeatures.geometryShader)
			return false;
	*/

	QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

	bool extensionsSupported = CheckDeviceExtensionSupport(physicalDevice);

	//Check for the swap chain 
	bool swapchainAdequate = false;
	if (extensionsSupported)
	{
		SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(physicalDevice);
		swapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
	}

	return indices.isComplete() && extensionsSupported && swapchainAdequate;


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



// -----------------------------Logical Device-------------------------------------

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

//---------------------------------Surface--------------------------------

void VulkanCore::CreateSurface()
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}
	Helper::Cout("Created Surface");
}


//----------------------------Swapchain-------------------------------

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


void VulkanCore::CreateImageViews()
{
	//Resize
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++) 
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];

		//How the image data should be interpreted (1D, 2D, 3D Textures and cube maps)
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;

		//Allows swizzling, for ex. mapping all channels to red
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		//Images Purpose
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create image views!");
		}
	}

	Helper::Cout("Created Image Views!");
}
