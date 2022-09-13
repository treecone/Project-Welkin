#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <stdexcept>
#include <stb_image.h>

struct Texture
{
	stbi_uc* pixels;
	int texWidth, texHeight, texChannels;
	VkDeviceSize imageSize;

	Texture(std::string PATH);

	~Texture();
};