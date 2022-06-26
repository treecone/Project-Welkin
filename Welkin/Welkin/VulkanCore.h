#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include "Helper.h"

class VulkanCore
{
public:
	VulkanCore(GLFWwindow* window);
	~VulkanCore();

private:
	GLFWwindow* window;
	VkInstance instance;


	void InitVulkan();
	void createInstance();
	void CheckAvaiableExtensions();
	bool CheckValidationLayerSupport();


	#pragma region ValidationLayers

		//Vector containing all the validation layers I want enabled
		const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
		};

		#ifdef NDEBUG
			const bool enableValidationLayers = false;
		#else
			const bool enableValidationLayers = true;
		#endif
	#pragma endregion

};
