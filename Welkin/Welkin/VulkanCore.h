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
#include "FileManager.h"
#include <string>
#include "Vertex.h"
#include "GameObject.h"
#include <algorithm>
#include <optional>
#include "Helper.h"
#include "UniformBufferObject.h"

const short MAX_FRAMES_IN_FLIGHT = 2;

class VulkanCore
{
public:

	VulkanCore(GLFWwindow* window, FileManager* fm, vector<GameObject*>* gameObjects, Camera* mainCamera);
	~VulkanCore();

	//Window resizing
	void SetWindowSize(int width, int height);
	bool framebufferResized = false;
	
	//Getters
	VkDevice* GetLogicalDevice();
	VkPhysicalDevice* GetPhysicalDevice();
	//Graphics = 0, Transfer = 1
	VkCommandPool* GetCommandPool(int type);
	Camera* GetCamera() { return this->mainCamera; };
	VkRenderPass* GetRenderPass() { return &this->renderPass; };
	VkInstance* GetInstance() { return &this->instance; };
	//0 - Graphics, 1 - presentation, 2 - transfer
	VkQueue* GetQueue(int type);
	size_t GetFramebufferCount() { return this->swapChainFramebuffers.size(); };
	VkFormat GetSwapchainImageFormat() { return this->swapChainImageFormat; };

	//Buffers
	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;

	//Drawing
	void DrawFrame();
	unsigned short currentFrame = 0;

	//Buffers
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;


private:

	vector<GameObject*>* gameObjects;
	FileManager* fm;
	Camera* mainCamera;

#pragma region Setup

	VkInstance instance;
	//Graphics card selected and being used
	VkPhysicalDevice physicalDevice;
	//Logical Device
	VkDevice device;
	//Pointer to the GLFW window we created
	GLFWwindow* window;


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



	void CreateSurface();
	void CreateSwapchain();
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void CreateImageViews();
	void RecreateSwapChain();
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

#pragma region Pipeline/Passes

	void CreateGraphicsPipeline();
	void CreateRenderPass();
	void CreateFrameBuffers();

	VkRenderPass renderPass;
	VkPipeline graphicsPipeline;
	VkPipelineLayout pipelineLayout;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	//Commands ---------------

	void CreateCommandPools();
	void CreateCommandBuffers(VkCommandPool pool);
	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	std::vector<VkCommandBuffer> mainCommandBuffers;
	VkCommandPool graphicsCommandPool;
	VkCommandPool transferCommandPool;

#pragma endregion

#pragma region DrawFrame and Sync Objects

	void CreateSyncObjects();
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
#pragma endregion

#pragma region Buffers

	vector<UniformBufferObject*> allUniformBufferObjects;

	struct PushConstants
	{
		//TODO transfer to storage buffer 
		//aka model->world matrix
		alignas(16) mat4 world; //64 bits
		alignas(16) mat4 worldInverseTranspose;

		//unsigned short materialID;
	};
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