#include "Texture.h"


Texture::Texture(std::string PATH)
{
	stbi_uc* pixels = stbi_load(PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels)
	{
		throw std::runtime_error("failed to load" + PATH + " texture image!");
	}
}

Texture::~Texture()
{
	delete pixels;
	pixels = nullptr;
}