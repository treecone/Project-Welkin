#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <stdexcept>
#include <stb_image.h>
#include "VulkanCore.h"
#include "Helper.h"

class Texture
{
public:
	stbi_uc* pixels;
	int texWidth, texHeight, texChannels;
	VkDeviceSize imageSize;
	short textureSpot;

	Texture(std::string PATH, VulkanCore* vCore, short textureSpot);
	~Texture();

	VkImageView GetTextureImageView() { return this->textureImageView; };

private:
	VulkanCore* vCore;

	void CreateBuffers();
	VkImage textureImage;
	VkImageView textureImageView;
	VkDeviceMemory textureImageMemory;
};