#include "UniformBufferObject.h"

//Good img explaining UBO's - https://vkguide.dev/docs/chapter-4/descriptors/

UniformBufferObject::UniformBufferObject(uBufferType bufferType, VkDevice* device, Camera* mainCamera, void (*CreateBuffer)(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&))
	: bufferType{ bufferType }, device{device}, mainCamera{mainCamera}, CreateBuffer{CreateBuffer}
{
	Helper::Cout("Creating Uniform Buffer");
	CreateDescriptorSetLayout(VK_SHADER_STAGE_VERTEX_BIT);
	CreateUniformBufers();
	CreateDescriptorPool();
	CreateDescriptorSets();
}

UniformBufferObject::~UniformBufferObject()
{
	vkDestroyDescriptorPool(*device, descriptorPool, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
	{
		vkDestroyBuffer(*device, uniformBuffers[i], nullptr);
		vkFreeMemory(*device, uniformBuffersMemory[i], nullptr);
	}

	vkDestroyDescriptorSetLayout(*device, descriptorSetLayout, nullptr);
}

//Stage flags -> What stage in the pipeline needs the data
void UniformBufferObject::CreateDescriptorSetLayout(const VkShaderStageFlags stageFlags)
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = stageFlags;
	uboLayoutBinding.pImmutableSamplers = nullptr;  //Used for image sampling - Optional

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;

	if (vkCreateDescriptorSetLayout(*device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void UniformBufferObject::CreateUniformBufers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	if (MAX_FRAMES_IN_FLIGHT != 2)
	{
		throw std::invalid_argument("UHOH");
	}

	uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
	{
		(*CreateBuffer)(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
	}
}

void UniformBufferObject::UpdateUniformBuffer(unsigned short currentFrame)
{
	switch (bufferType)
	{
	default:
	case(0):
		throw std::exception("Type of UBO not specified");
		break;
	case(1):
		mainCamera->UpdateProjectionMatrix();
		perFrameData.proj = mainCamera->GetProjection();

		mainCamera->UpdateViewMatrix();
		perFrameData.proj = mainCamera->GetView();

		void* data;
		vkMapMemory(*device, uniformBuffersMemory[currentFrame], 0, sizeof(UboPerFrame), 0, &data);
		memcpy(data, &perFrameData, sizeof(perFrameData));
		vkUnmapMemory(*device, uniformBuffersMemory[currentFrame]);
		break;
	case(2):
		//TODO implement this
		break;
	}
}

void UniformBufferObject::CreateDescriptorPool()
{
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

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

	int sizeOfCorrespondingStruct;

	switch (bufferType)
	{
	default:
	case(0):
		break;
	case(1):
		//Per frame
		sizeOfCorrespondingStruct = sizeof(UboPerFrame);
		break;
	case(2):
		//TODO implement this
		sizeOfCorrespondingStruct = sizeof(UboPerMaterial);
		break;
	}

	//TODO do this and make 1 only 1 buffer 
	//https://vkguide.dev/docs/chapter-4/descriptors_code_more/

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeOfCorrespondingStruct;

		VkWriteDescriptorSet descriptorWrite{};
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
	}
}

