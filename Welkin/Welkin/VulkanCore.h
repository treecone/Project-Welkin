#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

class VulkanCore
{
public:
	VulkanCore();
	~VulkanCore();
	void InitVulkan();
	VkInstance* GetInstance();
private:
	void CreateInstance();
	VkInstance* instance;

	//Validation Layers
	bool CheckValidationLayerSupport();
	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
};

