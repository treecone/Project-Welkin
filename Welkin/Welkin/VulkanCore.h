#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <algorithm> // Necessary for std::clamp
#include <set>
#include <string>
#include <optional>
#include "Helper.h"
#include "FileManager.h"

class VulkanCore
{
public:
	VulkanCore(GLFWwindow* window, FileManager* fileManager);
	~VulkanCore();

private:
	GLFWwindow* window;
	FileManager* fm;

	//Main Vulkan Instance
	VkInstance instance;

	//Graphics card selected and being used
	VkPhysicalDevice physicalDevice;

	//Logical Device
	VkDevice device;

	//Provides the ability to interface with the window system (aka GLFW)
	//Represents an abstract type of surface to present rednered images to
	VkSurfaceKHR surface;

	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;

	//Access images and do stuff with them
	std::vector<VkImageView> swapChainImageViews;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	//Graphics pipeline -----------
	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;



	//Queues ---------
	VkQueue graphicsQueue;
	VkQueue presentationQueue;

	//Instance --------------------------------
	void InitVulkan();
	void CreateInstance();
	void CheckAvaiableExtensions();
	bool CheckValidationLayerSupport();
	void CreateRenderPass();
	void CreateGraphicsPipeline();

	//Physical Device ---------------------------------

	struct QueueFamilyIndices
	{
		//Graphics queue and presentation could be the same, or not
		std::optional<unsigned int> graphicsFamily;
		std::optional<unsigned int> presentationFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value() && presentationFamily.has_value();
		}
	};

	void PickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice physicalDevice);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice);

	//Logical Device ------------------------------------------
	void CreateLogicalDevice();

	//Surface --------------------------------
	void CreateSurface();

	#pragma region Swap Chain
		void CreateSwapChain();
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

		//A list of required device extensions I want enabled
		const std::vector<const char*> deviceExtensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		struct SwapChainSupportDetails
		{
			//Min max number of images, max width and height
			VkSurfaceCapabilitiesKHR capabilities;
			//Pixel Format, Color space
			std::vector<VkSurfaceFormatKHR> formats;
			//Avaiable Presentation Modes
			std::vector<VkPresentModeKHR> presentModes;
		};

		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physicalDevice);

		//Next choose the right setting for the swap chain including
		//surface format (color depth) - presentation mode, and swap extent (resolution of the images)
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& avaiableFormats);
		//Verticle sync? Triple Buffering?
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	#pragma endregion

	//Images
	void CreateImageViews();

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
