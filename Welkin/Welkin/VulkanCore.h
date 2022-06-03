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
	void initVulkan();
	VkInstance* GetInstance();
private:
	void createInstance();
	bool CheckValidationLayerSupport();
	VkInstance* instance;
	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
};

