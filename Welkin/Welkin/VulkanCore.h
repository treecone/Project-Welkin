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

	//Logical Device
	VkDevice device;

	VkSwapchainKHR swapChain;

	//Writes the commands of want to exacute into a command buffer
	VkCommandBuffer commandBuffer;

	void RecordCommandBuffer(VkCommandBuffer commandBuffer, unsigned int imageIndex);

	//Queues ---------
	VkQueue graphicsQueue;
	VkQueue presentationQueue;


private:
	GLFWwindow* window;
	FileManager* fm;

	//Main Vulkan Instance
	VkInstance instance;

	//Graphics card selected and being used
	VkPhysicalDevice physicalDevice;



	//Provides the ability to interface with the window system (aka GLFW)
	//Represents an abstract type of surface to present rednered images to
	VkSurfaceKHR surface;

	
	std::vector<VkImage> swapChainImages;

	//Access images and do stuff with them
	std::vector<VkImageView> swapChainImageViews;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	//Graphics pipeline -----------
	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	std::vector<VkFramebuffer> swapChainFramebuffers;

	VkCommandPool commandPool;
	//Auto cleaned up when released from command pool


	//Instance --------------------------------
	void InitVulkan();
	void CreateInstance();
	void CheckAvaiableExtensions();
	bool CheckValidationLayerSupport();
	void CreateRenderPass();
	void CreateGraphicsPipeline();
	void CreateFrameBuffer();

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

	//Imageviews--------------------------
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

	//Command Pool ------------------------------
	void CreateCommandPool();
	void CreateCommandBuffer();
};

/*
Notes:
- VkInstance - root of everything. Represents a vulkan API context, u just need one. 
- VkPhysical Device - The GPU and its specs and capabilities.
- VkDevice - logical device. Actual GPU driver, how to communincate with the GPU.
- Swap Chain -  is created on a given size, and if the window resizes, you will have to recreate the swapchain again.
- Framebuffer attachments = render targets. The attachments specified during render pass creation are bound by wrapping them into a framebuffer object.
-- A framebuffer obj refrences all the VkImageView objects that represent the attachments.
- Render Pass = The set of attachments, the way they are used, and the rendering work that is performed using them.
- Subpass, using data from one pass in a subsequent one. Used to subdevide a single render pass into sperate logical phases. Aka a geometry pass (G:Buffer depth, normals, color), and a lighting pass (uses data from G-Buffer).
- Input attachments are image views that can be used for pixel local load operation inside a fragment shader.
-- This means that framebuffer attachments written in one subpass can be read from at the exact same pixel in further subpasses. 
-- The traditional way, without using input attachments would involve multiple passes, where the second pass would consume the attachment image views as e.g. combined images.
--On tile-based-renderer, which is pretty much anything on mobile, using input attachments is faster than the traditional multi-pass approach as pixel reads are fetched from tile memory instead of mainframebuffer, so if you target the mobile market it’s always a good idea to use input attachments instead of multiple passes when possible.
- All functions that record commands can be recognized by their vkCmd prefix. 
- Semaphores = is used to add order between queue operations. Used to both order work inside the same queue or between diffrerent queues.
- Subpass dependencies, which specify memory and execution dependencies between subpasses
-- The operations right before and after a subpass also count as implicit subpasses
*/