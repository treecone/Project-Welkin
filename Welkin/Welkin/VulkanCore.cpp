#include "VulkanCore.h"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VulkanCore::VulkanCore()
{
	Helper::Cout("Vulkan Core", true);
	InitVulkan();
}

VulkanCore::~VulkanCore()
{
	vkDestroyInstance(instance, nullptr);
	vkDestroyDevice(device, nullptr);
}

void VulkanCore::InitVulkan()
{
	CreateInstance();
	//SetupDebugManager(); (skipped this part)
	PickPhysicalDevice();
	CreateLogicalDevice();
}

//Creates the Vulkan Instance
void VulkanCore::CreateInstance()
{
	//Validation Layer check
	if (enableValidationLayers && !CheckValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}


	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Default Application";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Welkin Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;


	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	//Extensions
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	Helper::Cout("Extension count: " + std::to_string(glfwExtensionCount));

	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;


	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else 
	{
		createInfo.enabledLayerCount = 0;
	}


	//Checks current required extension to loaded extensions
	unsigned int extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());


	//Create the instance --------------------------
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}

void VulkanCore::CreateLogicalDevice()
{
	//Describes the number of queues we want for a single queue family
	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
	queueCreateInfo.queueCount = 1;

	/* The currently available drivers will only allow you to create 
	a small number of queues for each queue family and you don't really 
	need more than one. That's because you can create all of the command
	buffers on multiple threads and then submit them all at once on the main 
	thread with a single low-overhead call.*/

	//Assign priority to queue
	float queuePriority = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	//Device features
	VkPhysicalDeviceFeatures deviceFeatures{};

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

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	//Create the Graphics queue
	Helper::Cout("Graphics Queue Created");
	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
}

void VulkanCore::PickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	//No graphics cards found, exit
	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	//We found a graphics card, add it to this vector
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	#pragma region Check if the graphics card meets all requirements

		Helper::Cout("All physical devices");

		for (int i = 0; i < deviceCount; i++) 
		{
			if (IsDeviceSuitable(devices[i])) 
			{
				physicalDevice = devices[i];
				Helper::Cout("Graphics Card Chosen!");
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE) 
		{
			throw std::runtime_error("failed to find a suitable GPU!");
		}

	#pragma endregion

}

bool VulkanCore::IsDeviceSuitable(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	Helper::Cout("- ID " + deviceProperties.deviceID);
	Helper::Cout("- Name " + (std::string)deviceProperties.deviceName);


	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	//Do stuff if you want here, such as scoring the cards and picking the best one

	//Add more checks in the stuct if you need more then graphics

	if (!indices.isComplete())
	{
		FindQueueFamilies(device);
	}

	return indices.isComplete();
}

void VulkanCore::FindQueueFamilies(VkPhysicalDevice device)
{
	//Every operation in Vulkan requires queue - which belong to diffrent familes
	//Go through and find the index families this card supports
	
	//Logic to find queue family indices to populate struct 

	//Gets all the queueFamilies into a vector
	unsigned int queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
	
	Helper::Cout("Queue Family -----");

	//Here it checks if the vector contains certain queueFamilies
	for (int i = 0; i < queueFamilyCount; i++)
	{
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		Helper::Cout("- Queue Number: " + std::to_string(queueFamilies[i].queueCount));
		Helper::Cout("- Queue flags: " + std::to_string(queueFamilies[i].queueFlags));

	}

	/*
	//Example

	Queue number: 16
	Queue flags: {Graphics | Compute | Transfer | SparseBinding}
	Queue number: 1
	Queue flags: {Transfer}
	Queue number: 8
	Queue flags: {Compute}
	
	*/
	
}



//Validation Layers ------------------------------------


//Checks if all of the requiested layers are avaiable
bool VulkanCore::CheckValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}
