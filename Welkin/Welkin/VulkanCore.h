#pragma once

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <set>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>
#include <array>
#include <string>
#include <optional>
#include <algorithm>

#include "Helper.h"


const short MAX_FRAMES_IN_FLIGHT = 2;

class VulkanCore
{
public:

	VulkanCore(GLFWwindow* window);
	~VulkanCore();

	//Window resizing
	void SetWindowSize(int width, int height);
	bool framebufferResized = false;
	
	//Getters
	VkDevice* GetLogicalDevice();
	VkPhysicalDevice* GetPhysicalDevice();
	//Graphics = 0, Transfer = 1
	VkInstance* GetInstance() { return &this->instance; };
	//0 - Graphics, 1 - presentation, 2 - transfer
	VkQueue* GetQueue(int type);
	VkFormat GetSwapchainImageFormat() { return this->swapChainImageFormat; };
	std::vector<VkFramebuffer>* GetSwapchainFramebuffers() { return &this->swapChainFramebuffers; };
	size_t GetFramebufferCount() { return this->swapChainFramebuffers.size(); };
	//0 - Graphics, 1 - transfer
	VkCommandPool* GetCommandPool(int type);
	VkSwapchainKHR* GetSwapchain() { return &this->swapChain; };
	VkExtent2D* GetSwapchainExtent() { return &this->swapChainExtent; };
	VkPhysicalDeviceProperties GetPhysicalDeviceProperties();

	//Called from renderer
	void CreateFrameBuffers(VkRenderPass* renderPass = nullptr);
	void RecreateSwapChain();


#pragma region Buffers/Images

	void CreateBuffer(const VkDeviceSize size, const VkBufferUsageFlags usage, const VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void CopyBuffer(const VkBuffer srcBuffer, const VkBuffer dstBuffer, const VkDeviceSize size);
	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	uint32_t FindMemoryType(const uint32_t type_filter, const VkMemoryPropertyFlags properties);
	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	VkImageView CreateImageView(VkImage image, VkFormat format);
#pragma endregion


private:
#pragma region Setup

	VkInstance instance;
	//Graphics card selected and being used
	VkPhysicalDevice physicalDevice;
	//Logical Device
	VkDevice device;
	//Pointer to the GLFW window we created
	GLFWwindow* window;
	//Taken from renderer
	VkRenderPass* currentRenderPass;


	//Queues ---------
	VkQueue graphicsQueue;
	VkQueue presentationQueue;
	VkQueue transferQueue;

	void InitVulkan();
	void CreateInstance();
	void CheckAvaiableExtensions();
	bool CheckValidationLayerSupport();
	void PickPhysicalDevice();
	int RateDeviceSuitability(VkPhysicalDevice physicalDevice);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice);
	void CreateLogicalDevice();

	struct QueueFamilyIndices
	{
		std::optional<unsigned int> graphicsFamily;
		std::optional<unsigned int> presentFamily;
		std::optional<unsigned int> transferFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
		}
	};
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice);



	#pragma region ValidationLayers
		//Vector containing all the validation layers I want enabled
		const std::vector<const char*> WantedValidationLayers = { "VK_LAYER_KHRONOS_validation" };

	#ifdef NDEBUG
		const bool enableValidationLayers = false;
	#else
		const bool enableValidationLayers = true;
	#endif

	#pragma endregion

	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#pragma endregion

#pragma region Presentation

	//Provides the ability to interface with the window system (aka GLFW)
	//Represents an abstract type of surface to present rednered images to
	VkSurfaceKHR surface;
	//Holds the framebuffers as a queue of images that are waiting to be presented
	VkSwapchainKHR swapChain;
	//Holds the handles to the VkImages in the swap chain
	std::vector<VkImage> swapChainImages;
	//These are the attachments, put into the framebuffer
	std::vector<VkImageView> swapChainImageViews;
	//Holds specified color channels and types of the Swap Chain (aka R,G,B,A in 8 bit or...)
	VkFormat swapChainImageFormat;
	//Holds the actual resolution of the swap chain in pixels 
	VkExtent2D swapChainExtent;
	//Holds the attachments for the swinchain
	std::vector<VkFramebuffer> swapChainFramebuffers;



	void CreateSurface();
	void CreateSwapchain();
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void CreateImageViews();
	void CleanupSwapChain();


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

	void CreateCommandPools();
	VkCommandPool graphicsCommandPool;
	VkCommandPool transferCommandPool;

#pragma region Buffers
	VkCommandBuffer BeginSingleTimeCommands(VkCommandPool pool);
	void EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool);
	void CreateDescriptorsForTextures();
#pragma endregion


};

/*
Notes:
- VkInstance - root of everything. Represents a Vulkan API context, u just need one.
- VkPhysical Device - The GPU and its specs and capabilities.
- VkDevice - logical device. Actual GPU driver, how to communicate with the GPU.
- Swap Chain -  is created on a given size, and if the window resizes, you will have to recreate the swapchain again.
- Frame-buffer attachments = render targets. The attachments specified during render pass creation are bound by wrapping them into a framebuffer object.
-- A framebuffer obj references all the VkImageView objects that represent the attachments.
- Render Pass = The set of attachments, the way they are used, and the rendering work that is performed using them.
- Sub-pass, using data from one pass in a subsequent one. Used to subdivide a single render pass into sperate logical phases. Aka a geometry pass (G:Buffer depth, normals, color), and a lighting pass (uses data from G-Buffer).
- Input attachments are image views that can be used for pixel local load operation inside a fragment shader.
-- This means that framebuffer attachments written in one sub-pass can be read from at the exact same pixel in further subpasses.
-- The traditional way, without using input attachments would involve multiple passes, where the second pass would consume the attachment image views as e.g. combined images.
--On tile-based-renderer, which is pretty much anything on mobile, using input attachments is faster than the traditional multi-pass approach as pixel reads are fetched from tile memory instead of mainframebuffer, so if you target the mobile market it�s always a good idea to use input attachments instead of multiple passes when possible.
- All functions that record commands can be recognized by their vkCmd prefix.
- Semaphores = is used to add order between queue operations. Used to both order work inside the same queue or between diffrerent queues.
- Sub-pass dependencies, which specify memory and execution dependencies between sub-passes
-- The operations right before and after a sub-pass also count as implicit sub-passes

Think of Descriptor Set Layouts as a struct declaration whose members are pointers. The struct declaration describes the data your shader will need.

Think of Descriptor Sets as actual struct objects/variables themselves. You still need to fill out each member of the struct when you write the descripter set (like struct initialization).

Descriptor Pools are where you allocate your structs. It's like malloc() but only for the structs you choose.

*/