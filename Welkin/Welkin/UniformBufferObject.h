#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "Camera.h"
#include "Helper.h"
#include "VulkanCore.h"

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

enum uBufferType { nullBuffer = 0, perFrame = 1, perMaterial = 2 };

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

struct UboPerMaterial
{
	vec2 uvScale;
};
#pragma endregion

class UniformBufferObject
{
public:

	UniformBufferObject(uBufferType bufferType, VulkanCore* vCore, Camera* mainCamera);
	~UniformBufferObject();

	VkDescriptorSetLayout* GetDescriptorSetLayout() { return &descriptorSetLayout; };
	VkDescriptorSet GetDescriptorSet(unsigned short currentFrame) { return descriptorSets[currentFrame]; };

	void UpdateUniformBuffer(unsigned short currentFrame);
private:
	//Init
	VulkanCore* vCore;
	VkDevice* device;
	Camera* mainCamera;
	//void (*CreateBuffer)(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&);
	uBufferType bufferType;

	//TODO Find a better way to do this, like using a generic or smth
	//Structs
	UboPerFrame perFrameData;
	UboPerMaterial perMaterialData;

	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;

	//Multiple of these for each frame in flight
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<VkDescriptorSet> descriptorSets;


	void CreateDescriptorSetLayout(const VkShaderStageFlags stageFlags);
	void CreateUniformBufers();
	void CreateDescriptorPool();
	void CreateDescriptorSets();
};