#include "Renderer.h"

Renderer::Renderer(VulkanCore* VulkanCore, FileManager* fm)
{
	this->vCore = VulkanCore;
	this->fm = fm;
	Helper::Cout("Renderer", true);
	Init();
}


Renderer::~Renderer()
{
	vkDestroyCommandPool(*device, commandPool, nullptr);
	for (auto framebuffer : swapChainFrameBuffers) 
	{
		vkDestroyFramebuffer(*device, framebuffer, nullptr);
	}

	vkDestroyPipeline(*device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(*device, pipelineLayout, nullptr);
	vkDestroyRenderPass(*device, renderPass, nullptr);

	vkDestroySemaphore(*device, imageAvailableSemaphore, nullptr);
	vkDestroySemaphore(*device, renderFinishedSemaphore, nullptr);
	vkDestroyFence(*device, inFlightFence, nullptr);
}

void Renderer::Init()
{
	this->device = vCore->GetLogicalDevice();
	this->swapChainExtent = vCore->GetSwapChainExtent();
	this->swapChainImageViews = vCore->GetSwapChainImageViews();
	this->swapchain = vCore->GetSwapChain();
	this->graphicsQueue = vCore->GetQueue('0');
	this->presentationQueue = vCore->GetQueue('1');

	CreateRenderPass();
	CreateGraphicsPipeline();
	CreateFramebuffers();
	CreateCommandPool();
	CreateCommandBuffer();
	CreateSyncObjects();
}

/*
Steps:
- Wait for previous frame
- Acquire an image from the swap chain 
- Record a command buffer which draws the scene onto that image
- Submit the cmd buffer to a queue
- Present the swap chain

Semaphores - Handle operations in queues (cmd buffers, or functions), on the GPU
- Begins as unsiginaled 
- Does not block exacution
- When we submit a cmd, we can change and check for singletons

Fences - Similar Purpose, but is used for ordering the exacution on the CPU
- Also done during QueueSubmit, but uses vkWaitForFence to wait on CPU
- Does block exacution
- Starts Unsignaled
- Dont block host unless necessary 
*/
void Renderer::DrawFrame()
{
	//Waits for signaled------------------------
	vkWaitForFences(*device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
	vkResetFences(*device, 1, &inFlightFence);

	//Aquire Image from swap chain -------------------
	unsigned int imageIndex;
	vkAcquireNextImageKHR(*device, *swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	//Record Command Buffer -----------------------
	//This makes sure that the cmd buffer is able to be recorded
	vkResetCommandBuffer(commandBuffer, 0);
	RecordCommandBuffer(commandBuffer, imageIndex);

	//Queue Submission -------------------------
	#pragma region Submit Info
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		//Wait in the pipeline color stage for the imageAvaiableSemaphore
		VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		//Actual cmd buffer we want to submit
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		//What semaphores to signal once ^ are finished
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;
	#pragma endregion

		//Optional fence to be signaled after
	if (vkQueueSubmit(*graphicsQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	//Presentation --------------------------------
	//Submit the result back to the swap chain to have it eventually show up on the screen.

	//Presentation is configured through this
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	//What semaphores to wait for
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { *swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR(*presentationQueue, &presentInfo);
}

//Render pass -> Tells vulkan about the framebuffers attachments.
// "The attachments refrenced by the pipeline stages and their usage"
// Determines if we should clear new frame, and what to do with the data of the frames after being displyed
//Specifies how many color and depth buffers there will be, how many samplers to use for each of them, and how their contents will be handled
void Renderer::CreateRenderPass()
{
	#pragma region Color and Formats, Clearing New Frames
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = *vCore->GetSwapChainImageFormat();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		//Determines what should we do with the data in the attachment before rendering and after rendering
		//Options LoadOP: LOAD OP LOAD - perserve contents, LOAD OP CLEAR - Clear the values to a constant, LOAD OP DONT CARE - We dont care about them
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		//Options for storeOP: STORE OP STORE - Rendered contents will be stored in memory and be read later, STORE OP DONT CARE - Dont
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		//Now for the same thing, but with stencils. CHANGE THIS IF USING STENCILS
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		//Images are going to need to be transitioned, so this in the initial format
		//For example, Color attachment, present format, transfer format
		//Before Render pass
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		//After Render pass
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	#pragma endregion

	//Refrences the past passes, including dependancies and such
	#pragma region Subpasses

		//Create more of these if u need more then just a color attachment
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		//-------------------------------------

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
	#pragma endregion

	#pragma region Subpass Dependencies
			//Specify memory managment and execution dependencies between subpasses
			//Even if we only have one, the operations before and after this one subpass count 

			VkSubpassDependency dependency{};
			//SUBPASS_EXTERNAL = subpass before or after the render pass depending on if its src or dst 
			//dst must be > then src subpass (unless SUBPASS_EXTERNAL)
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;

			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;

			//Prevents the transition from happening until we actual want to start writing colors to it
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	#pragma endregion

	#pragma region RenderPassInfo
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;
	#pragma endregion




	if (vkCreateRenderPass(*device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}

	Helper::Cout("Created Render Pass!");
}

void Renderer::CreateGraphicsPipeline()
{
	VkShaderModule* vertShaderModule = &fm->allShaders.find("(C)SimpleShaderVert.spv")->second;
	VkShaderModule* fragShaderModule = &fm->allShaders.find("(C)SimpleShaderFrag.spv")->second;

	#pragma region Shader Stage Creation (programmable functions)
		//Actually assign the shaders to specific pipeline stages
		#pragma region Vertex 
			VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
			vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

			//Specifies the shader module containing the code, and the function to invoke
			vertShaderStageInfo.module = *vertShaderModule;
			//ITS POSSIBLE TO HAVE MULTIPLE SHADERS IN A MODULE BC OF THE ENTRY POINT
			vertShaderStageInfo.pName = "main";
		#pragma endregion

		#pragma region Fragment
			VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
			fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = *fragShaderModule;
			fragShaderStageInfo.pName = "main";
		#pragma endregion

	#pragma endregion

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	//Fixed Functions --------------------------------------------

	//Format of the vertex data passed to the vertex shader
	#pragma region Vertex Input
		//How vertex data will be passed to the vertex shader
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		//Spacing between data and per-vertex/per-instance 9instancing)
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional
	#pragma endregion

	//(1) What kind of geometry will be drawn from the verties
	//(2) What type of primitive restart will be enabled (such as triangle list, struo, and line strip)
	#pragma region Input Assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;
	#pragma endregion

	//Could be defined in dynamic states
	#pragma region ViewPort and Scissors
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		/* DONE IN DYNAMIC STATES
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapChainExtent->width;
		viewport.height = (float)swapChainExtent->height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;
		*/
	#pragma endregion

	//Depth, linewidth, face culling, depthclamp cull, polygon mode
	#pragma region Rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		//Fragments that are beyond the near and far planes are clamped vs discarding them
		rasterizer.depthClampEnable = VK_FALSE;
		//Geo passed through the rasterizer stage
		rasterizer.rasterizerDiscardEnable = VK_FALSE;

		//Polygon (Fill, Line, Point)
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;

		//Cull Front-back, or both, or neither
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	#pragma endregion

	//disabled for now...
	#pragma region Multisampling
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional
	#pragma endregion

	//need to implement...
	#pragma region Depth and Stencil Testing
		//VkPipelineDepthStencilStateCreateInfo stencilInfo{};
	#pragma endregion

	#pragma region Color Blending
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional
	#pragma endregion

	//States that can be changed during runtime
	#pragma region Dynamic States
		std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();
	#pragma endregion

	//Uniform values are variables that can be changed at drawing time. (etc: Transformation Matrix)
	//The uniform and push values refrenced by the shader that can be updated at draw time
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	if (vkCreatePipelineLayout(*device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}

	#pragma region Creating The actual pipeline
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
		pipelineInfo.pDynamicState = &dynamicState;

		pipelineInfo.layout = pipelineLayout;

		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;

		//Creating a new pipeline from a existing one - less expensive!
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional
	#pragma endregion

	//Could potentally create multuple graphis pipelines in one call. 
	if (vkCreateGraphicsPipelines(*device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	Helper::Cout("Created Graphics Pipeline!");

}

//Render pass expects a framebuffers with the same format as teh swap chain images
//The attachments that are specified during render pass creation are bound by wrapping them into framebuffers obects.
//Framebuffers refrence all vkImageView ibhects that represent the attachments
//Thus we need to create a framebuffer for all the images in the swap chain, using the one that corresponds to the retrieved image at drawing time
void Renderer::CreateFramebuffers()
{
	swapChainFrameBuffers.resize(swapChainImageViews->size());


	//Iterate through the image views and create framebuffers from them
	for (size_t i = 0; i < swapChainImageViews->size(); i++)
	{
		VkImageView attachments[] = { swapChainImageViews->at(i)};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapChainExtent->width;
		framebufferInfo.height = swapChainExtent->height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(*device, &framebufferInfo, nullptr, &swapChainFrameBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}

	Helper::Cout("Created Framebuffers");
}

//Command Pools are put onto indivisual queues
void Renderer::CreateCommandPool()
{
	VulkanCore::QueueFamilyIndices queueFamilyIndices = vCore->FindQueueFamilies(*vCore->GetPhysicalDevice());

	#pragma region Pool Info
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		//Change this if we are not recording a command buffer every frame
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	#pragma endregion

	if (vkCreateCommandPool(*device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create command pool!");
	}

	Helper::Cout("Created Command Pool!");
}

void Renderer::CreateCommandBuffer()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	//Change this to secondary if this command buffer is only called from another one
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(*device, &allocInfo, &commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}

	Helper::Cout("Created CommandBuffer!");
}

void Renderer::CreateSyncObjects()
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	//Starts Signaled
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateSemaphore(*device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(*device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
		vkCreateFence(*device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create semaphores!");
	}

	Helper::Cout("Created Sync Objects!");
}

//Writes commands we want to exacute into the command buffer
//image index is the index of the currently swapchain image we want to write to
void Renderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	#pragma region Command Buffer Basic Info
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//How to use the command buffer(etc. Secondary cmd buf, rerecorded right after exacuting once...)
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional, only relevent for secondary cmd buffers, what state to inherit
	#pragma endregion

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	#pragma region CmdBeginRenderPass
		VkRenderPassBeginInfo renderPassInfo{};
		//Render pass, and the attachments to bind
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFrameBuffers[imageIndex];

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = *swapChainExtent;

		//Based on VK_ATTACHMENT_LOAD_OP_CLEAR in render pass
		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		//Change this if commands will be exacuted from secondary cmd buffers
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	#pragma endregion

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

	#pragma region Dynamic States
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChainExtent->width);
		viewport.height = static_cast<float>(swapChainExtent->height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = *swapChainExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	#pragma endregion

	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to record command buffer!");
	}

	//Helper::Cout("Recorded Command Buffer!");


}

