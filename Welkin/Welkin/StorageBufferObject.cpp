#include "StorageBufferObject.h"

StorageBufferObject::StorageBufferObject(StorageBufferType storageType, VulkanCore* vCore, FileManager* fm, Camera* mainCamera):
	thisStorageType{storageType}, vCore{vCore}, fm{fm}, mainCamera{mainCamera}
{
	Helper::Cout("Creating Storage Buffer");
	device = vCore->GetLogicalDevice();

	CreateStorageBuffers();
	CreateDescriptorSetLayout();
	CreateDescriptorPool();
	CreateDescriptorSets();

}

StorageBufferObject::~StorageBufferObject()
{
	vkDestroyDescriptorPool(*device, descriptorPool, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroyBuffer(*device, storageBuffers[i], nullptr);
		vkFreeMemory(*device, storageBufferMemory[i], nullptr);
	}

	vkDestroyDescriptorSetLayout(*device, descriptorSetLayout, nullptr);
}

void StorageBufferObject::CreateStorageBuffers()
{
	if (thisStorageType == StorageBufferType::PER_TRANSFORM)
	{
		VkDeviceSize bufferSize = sizeof(Welkin_BufferStructs::PerTransformStruct) * Welkin_Settings::MAX_OBJECTS;

		storageBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		storageBufferMemory.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vCore->CreateBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, storageBuffers[i], storageBufferMemory[i]);
		}
	}
}


void StorageBufferObject::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	VkDescriptorSetLayoutCreateInfo layoutInfo{};

	switch (thisStorageType)
	{
		case(StorageBufferType::PER_TRANSFORM):

			uboLayoutBinding.binding = 0;
			uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = 1;
			layoutInfo.pBindings = &uboLayoutBinding;
			break;
	}

	if (vkCreateDescriptorSetLayout(*device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create storage descriptor set layout!");
	}
}

void StorageBufferObject::CreateDescriptorPool()
{
	VkDescriptorPoolSize poolSize{};
	VkDescriptorPoolCreateInfo poolInfo{};

	switch (thisStorageType)
	{
		case(StorageBufferType::PER_TRANSFORM):
			poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

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

void StorageBufferObject::CreateDescriptorSets()
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

	VkWriteDescriptorSet descriptorWrite{};
	VkDescriptorBufferInfo bufferInfo{};

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		switch (thisStorageType)
		{
			case(StorageBufferType::PER_TRANSFORM):

				bufferInfo.buffer = storageBuffers[i];
				bufferInfo.offset = 0;
				bufferInfo.range = (sizeof(Welkin_BufferStructs::PerTransformStruct)) * Welkin_Settings::MAX_OBJECTS;

				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = descriptorSets[i];
				descriptorWrite.dstBinding = 0;
				descriptorWrite.dstArrayElement = 0;

				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptorWrite.descriptorCount = 1;

				descriptorWrite.pBufferInfo = &bufferInfo;

				vkUpdateDescriptorSets(*device, 1, &descriptorWrite, 0, nullptr);
				break;
		}
	}
}

void StorageBufferObject::UpdateStorageBuffer(unsigned short currentFrame, vector<GameObject*>* allGameobjects)
{
	switch (thisStorageType)
	{
		case(StorageBufferType::PER_TRANSFORM):

			if (allGameobjects == nullptr)
			{
				throw std::runtime_error("inserted vector of gameobjects is nullptr in storageBufferObject");
			}


			void* data;
			vkMapMemory(*device, storageBufferMemory[currentFrame], 0, sizeof(Welkin_BufferStructs::PerTransformStruct), 0, &data);

			Welkin_BufferStructs::PerTransformStruct* allTransformsStruct = (Welkin_BufferStructs::PerTransformStruct*)data;
			for (int i = 0; i < allGameobjects->size(); i++)
			{
				allTransformsStruct[i].world = allGameobjects->at(i)->GetTransform()->GetWorldMatrix();
				allTransformsStruct[i].worldInverseTranspose = allGameobjects->at(i)->GetTransform()->GetWorldInverseTransposeMatrix();
			}
			
			vkUnmapMemory(*device, storageBufferMemory[currentFrame]);
			break;
	}
}