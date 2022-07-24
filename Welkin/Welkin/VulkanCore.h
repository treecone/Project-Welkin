#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <set>
#include <string>
#include <algorithm>
#include <optional>
#include "Helper.h"

class VulkanCore
{
public:
	VulkanCore(GLFWwindow* window);
	~VulkanCore();

	//Getters
	VkPhysicalDevice* GetPhysicalDevice();
	VkDevice* GetLogicalDevice();
	VkExtent2D* GetSwapChainExtent();
	VkFormat* GetSwapChainImageFormat();
	VkSwapchainKHR* GetSwapChain();
	//0 - Graphics, 1 - Presentation
	VkQueue* GetQueue(char queueNum = 0);
	std::vector<VkImage>* GetSwapChainImages();
	std::vector<VkImageView>* GetSwapChainImageViews();

	struct QueueFamilyIndices
	{
		std::optional<unsigned int> graphicsFamily;
		std::optional<unsigned int> presentFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice);


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


	VkSwapchainKHR swapChain;
	//Holds the handles to the VkImages in the swap chain
	std::vector<VkImage> swapChainImages;
	//Holds specified color channels and types of the Swap Chain (aka R,G,B,A in 8 bit or...)
	VkFormat swapChainImageFormat;
	//Holds the actual resolution of the swap chain in pixels
	VkExtent2D swapChainExtent;
	//These are the attachments, put into the framebuffer
	std::vector<VkImageView> swapChainImageViews;

	//Queues ---------
	VkQueue graphicsQueue;
	VkQueue presentationQueue;

	//Functions
	void InitVulkan();
	void CreateInstance();
	void CheckAvaiableExtensions();
	bool CheckValidationLayerSupport();
	void PickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice physicalDevice);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice);
	void CreateLogicalDevice();
	void CreateSurface();

	void CreateSwapchain();
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void CreateImageViews();

	#pragma region ValidationLayers

		//Vector containing all the validation layers I want enabled
		const std::vector<const char*> WantedValidationLayers = {"VK_LAYER_KHRONOS_validation"};

		#ifdef NDEBUG
			const bool enableValidationLayers = false;
		#else
			const bool enableValidationLayers = true;
		#endif

	#pragma endregion

	const std::vector<const char*> deviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	#pragma region Swapchain
		struct SwapchainSupportDetails 
		{
			//Basic surface capabilities (min/max # of images, min/max width height of images)
			VkSurfaceCapabilitiesKHR capabilities;
			//Pixel Format, Color Space
			std::vector<VkSurfaceFormatKHR> formats;
			//Avaiable Presentation Modes 
			std::vector<VkPresentModeKHR> presentModes;
		};

		SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice physicalDevice);
	#pragma endregion




};
