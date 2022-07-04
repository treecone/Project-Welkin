#include "VulkanCore.h"

VulkanCore::VulkanCore(GLFWwindow* window, FileManager* fileManager)
{
	Helper::Cout("Vulkan Core", true);
	this->window = window;
	this->fm = fileManager;
	InitVulkan();
}

VulkanCore::~VulkanCore()
{
	vkDestroyCommandPool(device, commandPool, nullptr);

	for (auto framebuffer : swapChainFramebuffers) 
	{
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}

	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);

	//Image views
	for (auto imageView : swapChainImageViews) 
	{
		vkDestroyImageView(device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(device, swapChain, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
}

void VulkanCore::InitVulkan()
{
	CreateInstance();
	CreateSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateSwapChain();
	CreateImageViews();
	CreateRenderPass();
	CreateGraphicsPipeline();
	CreateFrameBuffer();
	CreateCommandPool();
	CreateCommandBuffer();
}

//----------------------------Vulkan Instance Creation -----------------------------

//Creates the vulkan instance
void VulkanCore::CreateInstance()
{
	//Check for required extensions
	CheckAvaiableExtensions();

	//Check for required validation layers
	if (enableValidationLayers && !CheckValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	//Basic Information like application name and version
	#pragma region Application Info
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Welkin Project";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Welkin Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;
	#pragma endregion

	//Tells vulkan what global extensions and validation layers we want to use
	#pragma region VK Instance Create Info
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
	#pragma endregion

	#pragma region VK Instance - GLFW Extensions
		unsigned int glfwExtensionCount = 0;
		const char** glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;
	#pragma endregion

	#pragma region VK Instance - Validation Layers
		if (enableValidationLayers) 
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else 
		{
			createInfo.enabledLayerCount = 0;
		}

	#pragma endregion

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create instance!");
	}
	else
	{
		Helper::Cout("Instance Created!");
	}
}

//Retrieves a list of supported extensions before creating the instance
void VulkanCore::CheckAvaiableExtensions()
{
	unsigned int extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	Helper::Cout("Avaiable Extensions");

	for (const auto& extension : extensions) {
		Helper::Cout("-" + (std::string)extension.extensionName);
	}
}

//Checks if all the requested val layers (specified in the stuct) are avaiable
bool VulkanCore::CheckValidationLayerSupport()
{
	unsigned int validationLayerCount;
	vkEnumerateInstanceLayerProperties(&validationLayerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(validationLayerCount);
	vkEnumerateInstanceLayerProperties(&validationLayerCount, availableLayers.data());

	//Checking if we get all the layers we wanted
	#pragma region LayerCheck

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) 
			{
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				Helper::Cout("[Warning] Did not find all requested Validation Layers!");
				return false;
			}
		}
		Helper::Cout("Found all requested Validation Layers!");
		return true;
	#pragma endregion


}

// --------------------------Graphics Pipeline--------------------

void VulkanCore::CreateRenderPass()
{
	//telling Vulkan about the framebuffer attachments that will be used while rendering.
	
	#pragma region Color Attachment
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat;
		//multisampling
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

		//what to do with the data in the attachment before rendering and after rendering.
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		//If the contents of the swapchain will be stored for later use, or not
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		//Same thing but for stencils - change if using stencils
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		//Textures and framebuffers in Vulkan are represented by VkImage objects 
		// Common layouts = present, attachment, transfer
		//The initialLayout specifies which layout the image will have before the render pass begins
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		//The finalLayout specifies the layout to automatically transition to when the render pass finishes
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	#pragma endregion

	#pragma region Subpasses
		//ATTACHMENTS!
		
		// other types:
		//pInputAttachments: Attachments that are read from a shader
		//pResolveAttachments: Attachments used for multisampling color attachments
		//pDepthStencilAttachment : Attachment for depthand stencil data
		//pPreserveAttachments : Attachments that are not used by this subpass, but for which the data must be preserved
		
		
		//Use previous renders to do spicy things ;)
		VkAttachmentReference colorAttachmentRef{};
		//specifies which attachment to refrence by its index
		colorAttachmentRef.attachment = 0;
		//specifies which layout we would like the attachment to have during a subpass use
		//Automatically transitions the attachment to have during a subpass that uses this refrence.
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		//Specify this as a graphics subpass, could also be a compute one
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
	#pragma endregion

	#pragma region Subpass Dependency
		//https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Rendering_and_presentation
		VkSubpassDependency dependency{};
		//External -> the implict subpass before or after the render pass 
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		//What subpass we are refering to (0 being the first)
		dependency.dstSubpass = 0;

		//Specify the operations to wait on and the stages in which these operations occur
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;

		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	#pragma endregion


	#pragma region Render Pass
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		//Dependency 
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;
	#pragma endregion

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create render pass!");
	}
}

void VulkanCore::CreateGraphicsPipeline()
{
	Helper::Cout("Creating Graphics Pipeline!");
	Helper::Cout("[Note] Maybe make it so it doesn't load all shaders?");
	fm->LoadAllShaders(&device);

	VkShaderModule vertShaderModule = fm->allShaders.find("vert.spv")->second;
	VkShaderModule fragShaderModule = fm->allShaders.find("frag.spv")->second;

	//Assigning the shaders to specific pipeline stages
	#pragma region Programmable Pipeline Stage Create Info
		//Vertex -------------------------------------------------
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		//Fragment -------------------------------------------------
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

	#pragma endregion

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	#pragma region Fixed Pipelien Stage Create Info
		#pragma region Vertex Input and Input Assembly
		//Vertex Input---------------------------------------------
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		//This could be changed so data is stored per-vertex or per-instnace
		//Potentially change this for instanced rendering 
		vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

		//Input Assembly 
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		//Designate the topology, such as triangle list, strips, line list and more
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;
	#pragma endregion

		#pragma region Viewport and scissors
			//Describes the region of the framebuffer that the output will be rendered too
			//Could possibly do something such as a minimap here?
			//REALLY HANDY: https://vulkan-tutorial.com/images/viewports_scissors.png

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)swapChainExtent.width;
			viewport.height = (float)swapChainExtent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			//scissor rectangles define in which regions pixels will actually be stored.
			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = swapChainExtent;

			//Combine the two above into a viewport state
			//It is possible to use multiple viewports and scissor rectangles on some graphics cards,
			//	so its members reference an array of them. Using multiple requires enabling a GPU feature (see logical device creation).
			VkPipelineViewportStateCreateInfo viewportState{};
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.viewportCount = 1;
			viewportState.pViewports = &viewport;
			viewportState.scissorCount = 1;
			viewportState.pScissors = &scissor;
		#pragma endregion

		#pragma region Rasterizer
			//Takes geo -> pixels aka fragments
					//Also (depth tests, face culling, scissor test, draw whole shape or just edges (wireframe rendering))

			VkPipelineRasterizationStateCreateInfo rasterizer{};
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			//Fragments that are beyond the near and far planes are claped - opposed to discarding them
			rasterizer.depthClampEnable = VK_FALSE;
			//geometry never passes through the rasterizer stage. This basically disables any output to the framebuffer.
			rasterizer.rasterizerDiscardEnable = VK_FALSE;
			//draw: normal/wireframe/just vertices
			rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizer.lineWidth = 1.0f;

			rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

			//Can manually alerter the depth. Added subtract or do it based on fragments slope
			rasterizer.depthBiasEnable = VK_FALSE;
			rasterizer.depthBiasConstantFactor = 0.0f; // Optional
			rasterizer.depthBiasClamp = 0.0f; // Optional
			rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
		#pragma endregion

		#pragma region Multisampling (anti-aliasing)
			/*Because it doesn't need to run the fragment shader multiple times if only one 
				polygon maps to a pixel, it is significantly less expensive than simply 
				rendering to a higher resolution and then downscaling. */
			VkPipelineMultisampleStateCreateInfo multisampling{};
			multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisampling.minSampleShading = 1.0f; // Optional
			multisampling.pSampleMask = nullptr; // Optional
			multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
			multisampling.alphaToOneEnable = VK_FALSE; // Optional
		#pragma endregion

		#pragma region Depth and stencil testing
		//Not implemented yet
	#pragma endregion

		#pragma region Color Blending
			//Potentially mix the old and new color OR combine the old and new value using bitwise operations
			// contains the configuration per attached framebuffer (not global)
			VkPipelineColorBlendAttachmentState colorBlendAttachment{};
			//The resulting color is AND'd with the colorWriteMask to determine which channels are actually passed through.
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_FALSE;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional - how much of the first color we take
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional - how much of the last color we take
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional - how we use math on them
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
			/*Pusdo-code Example of whats going on:
			if (blendEnable) 
			{
				finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
				finalColor.a = (srcAlphaBlendFactor * newColor.a) <alphaBlendOp> (dstAlphaBlendFactor * oldColor.a);
			} 
			else 
			{
				finalColor = newColor;
			}

			finalColor = finalColor & colorWriteMask;
			*/

			VkPipelineColorBlendStateCreateInfo colorBlending{};
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			//Auto disables the method above if true
			colorBlending.logicOpEnable = VK_FALSE;
			colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
			colorBlending.attachmentCount = 1;
			colorBlending.pAttachments = &colorBlendAttachment;
			colorBlending.blendConstants[0] = 0.0f; // Optional
			colorBlending.blendConstants[1] = 0.0f; // Optional
			colorBlending.blendConstants[2] = 0.0f; // Optional
			colorBlending.blendConstants[3] = 0.0f; // Optional
		#pragma endregion

		#pragma region Dynamic State (change data ^ wo/ creating again)
		//Allows you to change some data (above) without recreating the entire pipeline
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_LINE_WIDTH
			//Blend constants here?
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();
	#pragma endregion

		//Transformation matrix and texture samplers
		#pragma region Pipeline Layout (alter shaders without recreating them)
			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = 0; // Optional
			pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
			pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
			pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
		#pragma endregion

	#pragma endregion

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}

	#pragma region Create Actual Graphics Pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;

		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr; // Optional
		pipelineInfo.layout = pipelineLayout;

		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;

		//Create a new graphics pipeline by deriving from another existing one
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

	#pragma endregion

	//Perameters here allow the use of fast creation of multiple pipelines using pipeline cache's
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}
	Helper::Cout("Created Graphics Pipeline!");

	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

void VulkanCore::CreateFrameBuffer()
{
	//FrameBuffer must be the same format as the swap chain images
	//Render pass expects framebuffers 

	//Resize the containers to hold all the frameBuffers 
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		VkImageView attachments[] = {swapChainImageViews[i]};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		//YOU CAN ONLY USE A FRAMEBUFFER WITH A RENDERPASS THAT IS COMPATIBLE WITH IT
		//	that means they yse the same number and type of attachments
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}

	Helper::Cout("Created All FrameBuffers!");

	

}



//------------------------------Physical Device---------------------------------------------

void VulkanCore::PickPhysicalDevice()
{
	#pragma region GettingAllCards

		unsigned int physicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

		if (physicalDeviceCount == 0)
			throw std::runtime_error("failed to find GPUs with Vulkan support!");

		std::vector<VkPhysicalDevice> devices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, devices.data());
	#pragma endregion

	for (int i = 0; i < devices.size(); i++)
	{
		if (isDeviceSuitable(devices[i]))
		{
			physicalDevice = devices[i];
			Helper::Cout("Device chosen!");
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("failed to find suitable GPU!");
	}
}

bool VulkanCore::isDeviceSuitable(VkPhysicalDevice physicalDevice)
{
	#pragma region Basic Properties and Features
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

		Helper::Cout("- Determining suitablity of card: " + (std::string)deviceProperties.deviceName);

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
	#pragma endregion

	//Can check for things such as if a card is a VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU 
	/*
	if (!deviceFeatures.geometryShader)
		return false;
	*/

	QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);
	bool extensionsSupport = CheckDeviceExtensionSupport(physicalDevice);

	//Check if at least one supported swap chain image format and one supported
	//presentation mode
	bool swapChainAdequate = false;

	if (extensionsSupport)
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return indices.isComplete() && extensionsSupport && swapChainAdequate;


}

VulkanCore::QueueFamilyIndices VulkanCore::FindQueueFamilies(VkPhysicalDevice physicalDevice)
{
	QueueFamilyIndices indices;

	//Logic here to find the queue family indices and to populate the struct

	#pragma region FindingQueues
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		for (unsigned int i = 0; i < queueFamilyCount; i++)
		{
			if (indices.isComplete())
			{
				break;
			}

			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

			if (presentSupport)
			{
				indices.presentationFamily = i;
			}

		}
	#pragma endregion

	return indices;
}



//Logical Device ------------------------------------------------------------------

void VulkanCore::CreateLogicalDevice()
{ 
	QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

	#pragma region Queue Create Info Structs

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<unsigned int> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentationFamily.value() };

		float queuePriority = 1.0f;
		for (unsigned int queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

	#pragma endregion

	#pragma region PhysicalDeviceFeatures
		VkPhysicalDeviceFeatures deviceFeatures{};

	#pragma endregion

	#pragma region Device Create Info
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}
	#pragma endregion

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create logical device!");
	}
	Helper::Cout("Logical Device Created!");

	#pragma region Queue Creation

		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		Helper::Cout("Graphics Queue Created!");
		vkGetDeviceQueue(device, indices.presentationFamily.value(), 0, &presentationQueue);
		Helper::Cout("Presentation Queue Created!");

	#pragma endregion

}

//Surface ---------------------------------------------------------

void VulkanCore::CreateSurface()
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}
}

//Swap Chain ------------------------------------------------------------------------

void VulkanCore::CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice);

	//Color depth - how much storage for each R, G, B, A
	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
	//Conditions for swapping images to the screen, aka swap mode (aka FIFO, IMMDIATE, MAILBOX)
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
	//Resolution of the images in the swap chain. Usually = width height of screen. However some computers have more pixels per resolution
	VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

	//how many images we would like to have in the swap chain
	//However, simply sticking to this minimum means that we may sometimes 
	//	have to wait on the driver to complete internal operations before
	//	we can acquire another image to render to. Therefore +1
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	#pragma region Create Info
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;

		/*It is also possible that you'll render images to a separate 
			image first to perform operations like post-processing. In that 
			case you may use a value like VK_IMAGE_USAGE_TRANSFER_DST_BIT 
			instead and use a memory operation to transfer the rendered image
			to a swap chain image.*/
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentationFamily.value() };

		if (indices.graphicsFamily != indices.presentationFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		//Additional image transforms
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

		//The compositeAlpha field specifies if the alpha channel should be used for blending with other windows in the window system.
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		//Better speed if clipped, but wont draw pixels if a window is blocking them
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

	#pragma endregion

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create swap chain!");
	}
	Helper::Cout("Swap Chain Created!");

	#pragma region Retrieve Handles
		//Query final number of images
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
	#pragma endregion

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

}

bool VulkanCore::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

VulkanCore::SwapChainSupportDetails VulkanCore::QuerySwapChainSupport(VkPhysicalDevice physicalDevice)
{
	SwapChainSupportDetails details;
	//First part - capabilites
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);
	//Second part - Surface formats
	unsigned int formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
	}

	//Third Part 
	unsigned int presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
	}

	return details; 
}

VkSurfaceFormatKHR VulkanCore::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats) 
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
		{
			return availableFormat;
		}
	}

	Helper::Cout("Perfered Color Format not found! Using Whatever's avaiable");
	return availableFormats[0];
}

VkPresentModeKHR VulkanCore::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	#pragma region Presentation Mode Options
		/*
		VK_PRESENT_MODE_IMMEDIATE_KHR: Images submitted by your application are transferred to the screen right away, which may result in tearing.
		VK_PRESENT_MODE_FIFO_KHR: The swap chain is a queue where the display takes an image from the front of the queue when the display is
			refreshed and the program inserts rendered images at the back of the queue. If the queue is full then the program has to wait.
			This is most similar to vertical sync as found in modern games. The moment that the display is refreshed is known as "vertical blank".
		VK_PRESENT_MODE_FIFO_RELAXED_KHR: This mode only differs from the previous one if the application is late and the queue was empty at
			the last vertical blank. Instead of waiting for the next vertical blank, the image is transferred right away when it finally
			arrives. This may result in visible tearing.
		VK_PRESENT_MODE_MAILBOX_KHR: This is another variation of the second mode. Instead of blocking the application when the queue
			is full, the images that are already queued are simply replaced with the newer ones. This mode can be used to render frames as
			fast as possible while still avoiding tearing, resulting in fewer latency issues than standard vertical sync. This is commonly
			known as "triple buffering", although the existence of three buffers alone does not necessarily mean that the framerate is unlocked.
		*/
	#pragma endregion

	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
		{
			//Change this if power efficency is a problem -> double buffering
			Helper::Cout("Perfered Presentation Mode found! Using: Triple Buffering");
			return availablePresentMode;
		}
	}

	Helper::Cout("Perfered Presentation Mode not found! Using Double Buffering");
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanCore::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	//the resolution of the swap chain images
	//Some displays have a higher resolution so we cant just use the screen coords

	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else 
	{
		int width, height;
		//query the resolution of the window in pixel
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

//Images ---------------------------------------------------------------------

void VulkanCore::CreateImageViews()
{
	swapChainImageViews.resize(swapChainImages.size());

	for (int i = 0; i < swapChainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];

		//Allows you to treat images as 1D, 2D, 3D textures and cube maps
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		//Describes what the image purpose is
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create image views!");
		}
	}

	Helper::Cout("Image Views Created!");
}

//Command stuff ------------------------------------------------

void VulkanCore::CreateCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(physicalDevice);
	#pragma region PoolInfo
		VkCommandPoolCreateInfo poolInfo{};

		/*
		Two possible flags:
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are rerecorded with new commands very often (may change memory allocation behavior)
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
		*/

		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	#pragma endregion

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create command pool!");
	}
}

void VulkanCore::CreateCommandBuffer()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	//speicifies if the allocated command buffers are primary or secondary 
	// - Primary (can be submitted to a queue for exacution, but cannot be called from other command buffers)
	// - Secondary (can not be submitted directly, but can be called from primary buffers)
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

void VulkanCore::RecordCommandBuffer(VkCommandBuffer commandBuffer, unsigned int imageIndex)
{
	//We always start recording a command buffer by calling vkBeginCommandBuffer
	#pragma region vkBeginCommandBuffer
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		/* FLAGS
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be rerecorded right after executing it once.
		VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: The command buffer can be resubmitted while it is also already pending execution.
		VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a secondary command buffer that will be entirely within a single render pass.
		*/
		beginInfo.flags = 0; // Optional
		//only relevant for secondary command buffers - specifies which state to inherit from
		beginInfo.pInheritanceInfo = nullptr; // Optional


	#pragma endregion

	//If a command buffer was already recorded once, then a call to vkBeginCommandBuffer will impliclity reset it. 
	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	//Drawing starts by beging the render pass with vkCmdBeginRenderPass
	#pragma region VkRenderPassBeginInfo
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		//Attachments to bind
		renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];

		//sizes
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;

		//Clear Values
		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

	#pragma endregion

	//Render Pass can now begin
	/*
	Last Value: 
	VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed.
	VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: The render pass commands will be executed from secondary command buffers.
	*/
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	//Basic Drawing commands -----------------------------

	//2nd - Specifies if the pipeline object is a graphics or compute pipeline.
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

	/*
	vertex count
	instance count (used for instancing)
	offset for first vertex
	offset for first instance
	*/
	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to record command buffer!");
	}
}