#include "UniformBufferObject.h"

//Good img explaining UBO's - https://vkguide.dev/docs/chapter-4/descriptors/

UniformBufferObject::UniformBufferObject(uBufferType bufferType, VulkanCore* vCore, FileManager* fm, Camera* mainCamera)
	: bufferType{ bufferType }, mainCamera{mainCamera}, vCore {vCore}, fm{fm}
{
	Helper::Cout("Creating Uniform Buffer");
	device = vCore->GetLogicalDevice();

	if (bufferType == bindlessTextures)
	{
		mainSampler = *fm->GetAllMaterials()->begin()->second->GetSampler();
		for (auto& tex : *fm->GetAllTextures())
		{
			imageViews.push_back(tex.second->GetTextureImageView());
		}

		if (imageViews.empty() || mainSampler == NULL)
		{
			throw std::runtime_error("Attempting to create descriptor set for textures, but either the sampler or image views are missing!");
		}
	}

	CreateDescriptorSetLayout();

	if (bufferType == perFrame)
	{
		CreateUniformBufers();
	}

	CreateDescriptorPool();
	CreateDescriptorSets();
}

UniformBufferObject::~UniformBufferObject()
{
	vkDestroyDescriptorPool(*device, descriptorPool, nullptr);

	if (bufferType != uBufferType::bindlessTextures)
	{
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyBuffer(*device, uniformBuffers[i], nullptr);
			vkFreeMemory(*device, uniformBuffersMemory[i], nullptr);
		}
	}

	vkDestroyDescriptorSetLayout(*device, descriptorSetLayout, nullptr);
}

//Stage flags -> What stage in the pipeline needs the data
void UniformBufferObject::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	VkDescriptorSetLayoutCreateInfo layoutInfo{};

	switch (bufferType)
	{
	default:
	case(0):
		throw std::exception("Type of UBO not specified");
		break;
	case(1):
		//Per Frame 
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;  //Used for image sampling - Optional

		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;
		break;
	case(2):
		//Textures
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		uboLayoutBinding.descriptorCount = imageViews.size();
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;  //Used for image sampling - Optional

		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;
		break;
	}

	if (vkCreateDescriptorSetLayout(*device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}

}

void UniformBufferObject::CreateUniformBufers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
	{
		vCore->CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
	}
}

void UniformBufferObject::UpdateUniformBuffer(unsigned short currentFrame)
{
	switch (bufferType)
	{
	default:
	case(0):
		//All UBO's are updated per frame, but some just end here so nothing is updated. Maybe change this?
		break;
	case(1):
		mainCamera->UpdateProjectionMatrix();
		perFrameData.proj = mainCamera->GetProjection();

		mainCamera->UpdateViewMatrix();
		perFrameData.view = mainCamera->GetView();

		void* data;
		vkMapMemory(*device, uniformBuffersMemory[currentFrame], 0, sizeof(UboPerFrame), 0, &data);
		memcpy(data, &perFrameData, sizeof(perFrameData));
		vkUnmapMemory(*device, uniformBuffersMemory[currentFrame]);
		break;
	case(3):
		break;
	}
}

void UniformBufferObject::CreateDescriptorPool()
{

	VkDescriptorPoolSize poolSize{};
	VkDescriptorPoolCreateInfo poolInfo{};

	switch (bufferType)
	{
	default:
	case(0):
		throw std::exception("Type of UBO not specified");
		break;
	case(1):
		//Per Frame
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		break;
	case(2):
		//Textures
		poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * imageViews.size());

		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		break;
	}

	if (vkCreateDescriptorPool(*device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void UniformBufferObject::CreateDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(*device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	size_t sizeOfCorrespondingStruct{};
	VkWriteDescriptorSet descriptorWrite{};
	VkDescriptorBufferInfo bufferInfo{};
	std::vector<VkDescriptorImageInfo> imageInfos(imageViews.size());


	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
	{
		switch (bufferType)
		{
		default:
		case(0):
			throw std::exception("Type of UBO not specified");
			break;
		case(1):
			sizeOfCorrespondingStruct = sizeof(UboPerFrame);

			bufferInfo.buffer = uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeOfCorrespondingStruct;

			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;

			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;

			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr; // Optional
			descriptorWrite.pTexelBufferView = nullptr; // Optional

			vkUpdateDescriptorSets(*device, 1, &descriptorWrite, 0, nullptr);
			break;

		case(2):

			for (uint32_t i = 0; i < imageViews.size(); ++i)
			{
				imageInfos[i].sampler = mainSampler;
				imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfos[i].imageView = imageViews[i];
			}

			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;

			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount = imageViews.size();

			descriptorWrite.pBufferInfo = nullptr;
			descriptorWrite.pImageInfo = imageInfos.data();
			descriptorWrite.pTexelBufferView = nullptr; 

			vkUpdateDescriptorSets(*device, 1, &descriptorWrite, 0, nullptr);
			break;
		}
	}
}

