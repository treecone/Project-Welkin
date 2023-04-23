#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "Camera.h"
#include "Helper.h"
#include "VulkanCore.h"
#include "FileManager.h"

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

//Make bindless textures so I dont eed a max
static const int MAX_TEXTURES_BOUND = 8;

enum UniformBufferType {PER_FRAME = 1, ALL_TEXTURES = 2 };

#pragma region UBO Structs
/*
Scalars have to be aligned by N (= 4 bytes given 32 bit floats).
A vec2 must be aligned by 2N (= 8 bytes)
A vec3 or vec4 must be aligned by 4N (= 16 bytes)
A nested structure must be aligned by the base alignment of its members rounded up to a multiple of 16.
A mat4 matrix must have the same alignment as a vec4.
*/

struct UboPerFrame
{
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};
#pragma endregion

class UniformBufferObject
{
public:

	UniformBufferObject(UniformBufferType bufferType, VulkanCore* vCore, FileManager* fm, Camera* mainCamera);
	~UniformBufferObject();

	VkDescriptorSetLayout* GetDescriptorSetLayout() { return &descriptorSetLayout; };
	VkDescriptorSet GetDescriptorSet(unsigned short currentFrame) { return descriptorSets[currentFrame]; };

	void UpdateUniformBuffer(unsigned short currentFrame);
private:
	//Init
	VulkanCore* vCore;
	VkDevice* device;
	Camera* mainCamera;
	UniformBufferType bufferType;
	FileManager* fm;

	//TODO Find a better way to do this, like using a generic or smth
	//Structs
	UboPerFrame perFrameData;

	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;

	//Multiple of these for each frame in flight
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<VkDescriptorSet> descriptorSets;

	//Imported from VulkanCore
	std::vector<VkImageView> imageViews;
	VkSampler mainSampler;


	void CreateDescriptorSetLayout();
	void CreateUniformBufers();
	void CreateDescriptorPool();
	void CreateDescriptorSets();
};