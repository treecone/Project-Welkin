#include "VulkanCore.h"

VulkanCore::VulkanCore(GLFWwindow* window)
{
	Helper::Cout("Vulkan Core", true);
	this->window = window;
	InitVulkan();
}

VulkanCore::~VulkanCore()
{
	vkDestroyDevice(device, nullptr);
	vkDestroyInstance(instance, nullptr);
}

void VulkanCore::InitVulkan()
{
	CreateInstance();
	PickPhysicalDevice();
	CreateLogicalDevice();
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
		appInfo.pApplicationName = "Welkin Proejct";
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
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
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

	Helper::Cout("Avaiable Extensions");

	for (const auto& extension : extensions) {
		Helper::Cout("-" + (std::string)extension.extensionName);
	}
}

//Checks if all the requested val layers (specified in the stuct) are avaiable
bool VulkanCore::CheckValidationLayerSupport()
{
	unsigned int validationLayerCount;
	vkEnumerateInstanceLayerProperties(&validationLayerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(validationLayerCount);
	vkEnumerateInstanceLayerProperties(&validationLayerCount, availableLayers.data());

	//Checking if we get all the layers we wanted
	#pragma region LayerCheck

		for (const char* layerName : validationLayers) {
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

	return indices.isComplete();


}

VulkanCore::QueueFamilyIndices VulkanCore::FindQueueFamilies(VkPhysicalDevice physicalDevice)
{
	QueueFamilyIndices indices;

	//Logic here to find the queue family indices and to populate the struct

	#pragma region FindingQueues
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		for (int i = 0; i < queueFamilyCount; i++)
		{
			if (indices.isComplete())
			{
				break;
			}

			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}
		}
	#pragma endregion

	return indices;
}



//Logical Device ------------------------------------------------------------------

void VulkanCore::CreateLogicalDevice()
{
	QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

	#pragma region Queue Create Info Structs
		//Describes the amount of queues we want for each family
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
		queueCreateInfo.queueCount = 1;

		float queuePriority = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePriority;
	#pragma endregion

	#pragma region PhysicalDeviceFeatures
		VkPhysicalDeviceFeatures deviceFeatures{};

	#pragma endregion

	#pragma region Device Create Info
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;

		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = 0;

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}
	#pragma endregion

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create logical device!");
	}
	Helper::Cout("Logical Device Created!");

	#pragma region Queue Creation
		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		Helper::Cout("Graphics Queue Created!");
	#pragma endregion

}
