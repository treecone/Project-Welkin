#include "Renderer.h"

Renderer::Renderer(VulkanCore* vCore, FileManager* fm, Camera* mainCamera, vector<GameObject*>* gameObjects) : vCore{ vCore }, mainCamera{ mainCamera }, gameObjects{ gameObjects }, fm{fm}
{
	this->device = vCore->GetLogicalDevice();

	Helper::Cout("Renderer", true);

	CreateRenderPass();
	allUniformBufferObjects.push_back(new UniformBufferObject(UniformBufferType::PER_FRAME, vCore, fm, this->mainCamera));
	allUniformBufferObjects.push_back(new UniformBufferObject(UniformBufferType::ALL_TEXTURES, vCore, fm, this->mainCamera));

	allStorageBufferObjects.push_back(new StorageBufferObject(StorageBufferType::PER_TRANSFORM, vCore, fm, this->mainCamera));

	CreateGraphicsPipeline();
	vCore->CreateFrameBuffers(&renderPass);

	CreateCommandBuffers(*vCore->GetCommandPool(0));
	CreateSyncObjects();
}

Renderer::~Renderer()
{
	for (auto& UBO : allUniformBufferObjects)
	{
		delete UBO;
	}

	//Sync Objects 
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(*device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(*device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(*device, inFlightFences[i], nullptr);
	}


	vkDestroyPipeline(*device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(*device, pipelineLayout, nullptr);
	vkDestroyRenderPass(*device, renderPass, nullptr);
}

void Renderer::CreateGraphicsPipeline()
{
	VkShaderModule* vertShaderModule = fm->FindShaderModule("(C)SimpleShaderVert.spv");
	VkShaderModule* fragShaderModule = fm->FindShaderModule("(C)SimpleShaderFrag.spv");

	#pragma region Shader Stage Creation

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = *vertShaderModule;
		vertShaderStageInfo.pName = "main";
		vertShaderStageInfo.pSpecializationInfo = nullptr; //Optional

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = *fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	#pragma endregion

	#pragma region Vertex Shader Info

		//Discribes the format of the vertex data that will be passed to the vertex shader
		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	#pragma endregion

	#pragma region Input Assembly

		//What type of geometry will be drawn, and if primitive restart should be enabled
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;
	#pragma endregion

	#pragma region Viewport and Scissors

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

	#pragma region Rasterizer
		//Depth, linewidth, face culling, depthclamp cull, polygon mode
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
		//TODO change this to Vk cull mode back bit 
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
	#pragma endregion

	#pragma region Multisampling

		//TODO implement
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional
	#pragma endregion

	#pragma region Depth and Stencil

		//TODO implement
		VkPipelineDepthStencilStateCreateInfo depthInfo{};
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

	#pragma region DynamicStates
		//States that can be changed in runtime in the pipeline
		std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();
	#pragma endregion

	#pragma region Pipeline Layout

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		vector<VkDescriptorSetLayout> allDescriptorLayouts;
		for (auto& UBO : allUniformBufferObjects)
		{
			allDescriptorLayouts.push_back(*UBO->GetDescriptorSetLayout());
		}
		for (auto& SBO : allStorageBufferObjects)
		{
			allDescriptorLayouts.push_back(*SBO->GetDescriptorSetLayout());
		}
		pipelineLayoutInfo.setLayoutCount = allDescriptorLayouts.size();
		pipelineLayoutInfo.pSetLayouts = allDescriptorLayouts.data();

		VkPushConstantRange range{};
		range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		range.offset = 0;
		range.size = sizeof(Welkin_BufferStructs::PushConstant);

		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &range;

		if (vkCreatePipelineLayout(*device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}

	#pragma endregion

	#pragma region Creating the Pipeline

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;

		//Use all the data above 
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

		if (vkCreateGraphicsPipelines(*device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create graphics pipeline!");
		}

	#pragma endregion

}

//Tells vulkan about the framebuffer attachments (aka color and depth buffers)
//Includes what to do with data before and after rendering
void Renderer::CreateRenderPass()
{
	#pragma region Color and Formats, Clearing New Frames
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = vCore->GetSwapchainImageFormat();
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

		//Create more of these if u need more then just a color attachment in this renderpass
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	#pragma endregion

	#pragma region Subpasses

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = nullptr; //TODO add
	#pragma endregion

	#pragma region Subpass Dependencies
		//Making it so the render pass waits at the VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT stage 
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0; //refers to first subpass

		//specify the operations to wait on and the stages in which these operations occur.
		//These settings will prevent the transition from happening until it's actually necessary (and allowed): when we want to start writing colors to it.
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	#pragma endregion

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(*device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}

}

void Renderer::CreateCommandBuffers(const VkCommandPool pool)
{
	mainCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = pool;
	//Specifies if the allocated command buffers are primary or secondary command buffers
	//aka Submitted to queue, or called from other cmd buffer
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)mainCommandBuffers.size();

	if (vkAllocateCommandBuffers(*device, &allocInfo, mainCommandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}

	Helper::Cout("Created Command Buffers");

}

void Renderer::RecordCommandBuffer(const VkCommandBuffer commandBuffer, const uint32_t imageIndex)
{
	#pragma region Begin Cmd Buffer

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//How to use the cmd buffer (aka rerecoreded after exacution, secondary cmd buffer, resubmitted while pending execution)
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}
	#pragma endregion

	#pragma region Begin Render-Pass
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		//.framebuffer = swapChainFramebuffers[imageIndex];
		renderPassInfo.framebuffer = vCore->GetSwapchainFramebuffers()->at(imageIndex);
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = *vCore->GetSwapchainExtent();

		//What to reset the color with
		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		//Either Inline (no secondary cmd buffers) or subpass (cmds will be executed from a secondary cmd buffer)
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	#pragma endregion

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

	#pragma region Dynamic States Setting
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(vCore->GetSwapchainExtent()->width);
		viewport.height = static_cast<float>(vCore->GetSwapchainExtent()->height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = *vCore->GetSwapchainExtent();
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	#pragma endregion

	//Used for not updating the vertex and index buffers every frame
	uint32_t lastIndicesSize = 0;

	#pragma region Binding Buffers

		//Uniform Buffer Objects 
		vector<VkDescriptorSet> allCurrentFrameDescriptorSets;
		for (auto& UBO : allUniformBufferObjects)
		{
			allCurrentFrameDescriptorSets.push_back(UBO->GetDescriptorSet(currentFrame));
		}
		for (auto& SBO : allStorageBufferObjects)
		{
			allCurrentFrameDescriptorSets.push_back(SBO->GetDescriptorSet(currentFrame));
		}

		//Binds all descriptor sets
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, allUniformBufferObjects.size() + allStorageBufferObjects.size(), allCurrentFrameDescriptorSets.data(), 0, nullptr);

		for (unsigned int i = 0; i < gameObjects->size(); i++)
		{
			//Push Constant
			Welkin_BufferStructs::PushConstant push{};
			/*push.world = gameObjects->at(i)->GetTransform()->GetWorldMatrix();
			push.worldInverseTranspose = gameObjects->at(i)->GetTransform()->GetWorldInverseTransposeMatrix();*/
			push.tempData = i;
			vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Welkin_BufferStructs::PushConstant), &push);

			const auto newIndicesSize = (gameObjects->at(i)->GetMesh()->GetIndeicesSize());
			//string newMaterialName = gameObjects->at(i)->GetMaterial()->GetMaterialName();

			if (newIndicesSize != lastIndicesSize)
			{
				//New Mesh, bind new vertex and index buffers

				const VkBuffer vertexBuffers[] = { *gameObjects->at(i)->GetMesh()->GetVertexBuffer() };
				constexpr  VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
				vkCmdBindIndexBuffer(commandBuffer, *gameObjects->at(i)->GetMesh()->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

				lastIndicesSize = newIndicesSize;
			}

			//TODO optimize this, create a single buffer for all meshes and then use offsets

			vkCmdDrawIndexed(commandBuffer, newIndicesSize, 1, 0, 0, 0);
		}
	#pragma endregion

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer!");
	}

}

void Renderer::DrawFrame()
{
	/*
	Wait for the previous frame to finish
	Acquire an image from the swap chain
	Record a command buffer which draws the scene onto that image
	Submit the recorded command buffer
	Present the swap chain image
	*/

	//Wait until the previous frame has finished, aka waits for signaled
	vkWaitForFences(*device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	//Aquire img from swap chain to draw to
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(*device, *vCore->GetSwapchain(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		vCore->RecreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	//Updating the Uniform Buffer Objects 
	for (auto& UBO : allUniformBufferObjects)
	{
		UBO->UpdateUniformBuffer(currentFrame);
	}
	for (auto& SBO : allStorageBufferObjects)
	{
		SBO->UpdateStorageBuffer(currentFrame, gameObjects);
	}

	//Sets fence(s) to unsignaled state
	vkResetFences(*device, 1, &inFlightFences[currentFrame]);

	//Reset and record cmd buffer
	vkResetCommandBuffer(mainCommandBuffers[currentFrame], 0);
	RecordCommandBuffer(mainCommandBuffers[currentFrame], imageIndex);

	#pragma region Submit Cmd Buffer

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		//Use the img available semaphore to wait for images in the Ouput Bit Color stage
		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		//What cmd buffer to submit
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mainCommandBuffers[currentFrame];

		//What sempaphores to singal once finshed
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		//inFlightFence is signaled after cmd buffer finishes exacution
		if (vkQueueSubmit(*vCore->GetQueue(0), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}
	#pragma endregion	

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { *vCore->GetSwapchain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(*vCore->GetQueue(1), &presentInfo);


	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vCore->framebufferResized)
	{
		vCore->framebufferResized = false;
		vCore->RecreateSwapChain();
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::CreateSyncObjects()
{
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(*device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(*device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(*device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
		{

			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}

	Helper::Cout("Created Sync Objects");
}