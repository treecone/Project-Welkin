#pragma once
#include "VulkanCore.h"
#include "FileManager.h"
#include "Camera.h"
#include "Helper.h"
#include "GameObject.h"

enum StorageBufferType { PER_TRANSFORM = 0 };

class StorageBufferObject
{

public:
	StorageBufferObject(StorageBufferType storageType, VulkanCore* vCore, FileManager* fm, Camera* mainCamera);
	~StorageBufferObject();

	VkDescriptorSetLayout* GetDescriptorSetLayout() { return &descriptorSetLayout; };
	VkDescriptorSet GetDescriptorSet(unsigned short currentFrame) { return descriptorSets[currentFrame]; };
	void UpdateStorageBuffer(unsigned short currentFrame, vector<GameObject*>* allGameobjects);

private:
	VulkanCore* vCore;
	VkDevice* device;
	Camera* mainCamera;
	StorageBufferType thisStorageType;
	FileManager* fm;

	//Storage Stuff
	vector<VkBuffer> storageBuffers;
	std::vector<VkDeviceMemory> storageBufferMemory;

	//Descriptor Stuff
	std::vector<VkDescriptorSet> descriptorSets;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;


	void CreateDescriptorSetLayout();
	void CreateStorageBuffers();
	void CreateDescriptorPool();
	void CreateDescriptorSets();
};

