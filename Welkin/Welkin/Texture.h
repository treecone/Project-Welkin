#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <stdexcept>
#include <stb_image.h>
#include "VulkanCore.h"

class Texture
{
public:
	stbi_uc* pixels;
	int texWidth, texHeight, texChannels;
	VkDeviceSize imageSize;

	Texture(std::string PATH, VulkanCore* vCore);
	~Texture();

private:
	void CreateBuffers(VulkanCore* vCore);

	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
};