#include "VulkanCore.h"

VulkanCore::VulkanCore(GLFWwindow* window)
{
	Helper::Cout("Vulkan Core", true);
	this->window = window;
	InitVulkan();
}

VulkanCore::~VulkanCore()
{
	vkDestroyInstance(instance, nullptr);
}

void VulkanCore::InitVulkan()
{
	createInstance();
}

//Creates the vulkan instance
void VulkanCore::createInstance()
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
