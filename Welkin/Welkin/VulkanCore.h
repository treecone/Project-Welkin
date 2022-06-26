#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <optional>
#include "Helper.h"

class VulkanCore
{
public:
	VulkanCore(GLFWwindow* window);
	~VulkanCore();

private:
	GLFWwindow* window;
	//Main Vulkan Instance
	VkInstance instance;
	//Graphics card selected and being used
	VkPhysicalDevice physicalDevice;
	//Logical Device
	VkDevice device;
	//Provides the ability to interface with the window system (aka GLFW)
	//Represents an abstract type of surface to present rednered images to
	VkSurfaceKHR surface;

	//Queues ---------
	VkQueue graphicsQueue;

	//Instance --------------------------------
	void InitVulkan();
	void CreateInstance();
	void CheckAvaiableExtensions();
	bool CheckValidationLayerSupport();

	//Physical Device ---------------------------------

	struct QueueFamilyIndices
	{
		std::optional<unsigned int> graphicsFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value();
		}
	};

	void PickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice physicalDevice);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice);

	//Logical Device ------------------------------------------

	void CreateLogicalDevice();

	#pragma region ValidationLayers

		//Vector containing all the validation layers I want enabled
		const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

		#ifdef NDEBUG
			const bool enableValidationLayers = false;
		#else
			const bool enableValidationLayers = true;
		#endif
	#pragma endregion

};
