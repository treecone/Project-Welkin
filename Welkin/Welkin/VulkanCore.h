﻿#pragma once

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
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
#include <algorithm>
#include <optional>
#include "Helper.h"
#define GLM_ENABLE_EXPERIMENTAL

const short MAX_FRAMES_IN_FLIGHT = 2;


class VulkanCore
{
public:

	VulkanCore(GLFWwindow* window, FileManager* fm);
	~VulkanCore();
	void DrawFrame();
	void SetWindowSize(int width, int height);
	VkDevice* GetLogicalDevice();

	uint32_t currentFrame = 0;

private:

	FileManager* fm;

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

	void InitVulkan();
	void CreateInstance();
	void CheckAvaiableExtensions();
	bool CheckValidationLayerSupport();
	void PickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice physicalDevice);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice);
	void CreateLogicalDevice();

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
	//Frame Buffers of the VkImages in the swap chain 
	std::vector<VkFramebuffer> swapChainFrameBuffers;


	void CreateSurface();
	void CreateSwapchain();
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void CreateImageViews();
	void CreateFramebuffers();
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

#pragma region Graphics Pipelines/Render Passes/Pipeline
	

	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	//Command Stuff
	std::vector<VkCommandBuffer> commandBuffers;
	VkCommandPool commandPool;

	//Sync Objects
	std::vector <VkSemaphore> imageAvailableSemaphores;
	std::vector <VkSemaphore> renderFinishedSemaphores;
	std::vector <VkFence> inFlightFences;

	bool frameBufferResized = false;

	void CreateRenderPass();
	void CreateGraphicsPipeline();
	void CreateCommandPool();
	void CreateCommandBuffers();
	void CreateSyncObjects();
	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

#pragma endregion

#pragma region Buffers and such

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	void CreateVertexBuffer();
	void CreateIndexBuffer();
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	//Descriptors --------------

	void CreateDescriptorSetLayout();
	void UpdateUniformBuffer(uint32_t currentImage);
	void CreateDescriptorPool();
	void CreateDescriptorSets();
	void CreateUniformBuffers();


	//Multiple ones, bc mulitple frames may be in flight at the same time 
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;

	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	/*
	- Allignment: (bytes)
	-- Scalars : 4
	-- Vec2 : 8
	-- Vec3/4 : 16
	-- Nested : 16
	-- Mat4 : 16
	*/

	struct UniformBufferObject {
		alignas (16) glm::mat4 model;
		alignas (16) glm::mat4 view;
		alignas (16) glm::mat4 proj;
	};

#pragma endregion

#pragma region Image
	void CreateTextureImage();
	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	VkCommandBuffer BeginSingleTimeCommands();
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
	void CreateTextureImageView();
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	void CreateTextureSampler();


	VkImage textureImage;
	VkSampler textureSampler;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;


	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	void CreateDepthResources();
	VkFormat FindDepthFormat();
	bool HasStencilComponent(VkFormat format);
	//Finds the correct format for the depth buffer
	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
#pragma endregion

#pragma region Model
	const std::string MODEL_PATH = "Models/viking_room.obj";
#pragma endregion


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
--On tile-based-renderer, which is pretty much anything on mobile, using input attachments is faster than the traditional multi-pass approach as pixel reads are fetched from tile memory instead of mainframebuffer, so if you target the mobile market it�s always a good idea to use input attachments instead of multiple passes when possible.
- All functions that record commands can be recognized by their vkCmd prefix.
- Semaphores = is used to add order between queue operations. Used to both order work inside the same queue or between diffrerent queues.
- Subpass dependencies, which specify memory and execution dependencies between subpasses
-- The operations right before and after a subpass also count as implicit subpasses
*/