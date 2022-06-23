#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <optional>
#include "Helper.h"

struct QueueFamilyIndices
{
	std::optional<unsigned int> graphicsFamily;

	bool isComplete()
	{
		return graphicsFamily.has_value();
	}
};


class VulkanCore
{
public:
	//Directs graphic commands to the hardware, LogicalDevice.
	VkDevice device;
	//The Vulkan Instance
	VkInstance instance;
	//Actual physical device aka the g card
	VkPhysicalDevice physicalDevice;
	//One of the types of queues, created in help from the index we got earlier
	VkQueue graphicsQueue;
	//Stuct of the locations of the queue families we need
	QueueFamilyIndices indices;
	// represents an abstract type of surface to present rendered images to
	VkSurfaceKHR surface;




	VulkanCore();
	~VulkanCore();
	void InitVulkan();
private:
	void CreateInstance();
	void CreateLogicalDevice();
	void PickPhysicalDevice();
	bool IsDeviceSuitable(VkPhysicalDevice device);
	void FindQueueFamilies(VkPhysicalDevice device);



	//Validation Layers
	bool CheckValidationLayerSupport();
	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
};
