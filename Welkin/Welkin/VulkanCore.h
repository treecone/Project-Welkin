#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <set>
#include <glm/glm.hpp>
#include <array>
#include "FileManager.h"
#include <string>
#include <algorithm>
#include <optional>
#include "Helper.h"

class VulkanCore
{
public:

	VulkanCore(GLFWwindow* window, FileManager* fm);
	~VulkanCore();
	void DrawFrame();
	void SetWindowSize(int width, int height);
	VkDevice* GetLogicalDevice();

	uint32_t currentFrame = 0;
	const short MAX_FRAMES_IN_FLIGHT = 2;

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

	void CreateVertexBuffer();
	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	struct Vertex 
	{
		glm::vec2 pos;
		glm::vec3 color;


		//Describes at which rate to load data from memory throughout the vertices
		static VkVertexInputBindingDescription getBindingDescription() 
		{
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDescription;
		}

		//How to handle the input 
		static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() 
		{
			std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			return attributeDescriptions;
		}
	};

	const std::vector<Vertex> vertices = 
	{
		{{0.0f, -0.5f}, {1.0f, 0.0f, 1.0f}},
		{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
		{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
	};

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