#include "Texture.h"

Texture::Texture(std::string PATH, VulkanCore* vCore)
{
	stbi_uc* pixels = stbi_load(PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels)
	{
		throw std::runtime_error("failed to load" + PATH + " texture image!");
	}

	CreateBuffers(vCore);
}

Texture::~Texture()
{

}

void Texture::CreateBuffers(VulkanCore* vCore)
{
	VkDevice* device = vCore->GetLogicalDevice();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	vCore->CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(*device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(*device, stagingBufferMemory);

	stbi_image_free(pixels);

	//------------Creating the acutal img and buffers ------------

	vCore->CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
}