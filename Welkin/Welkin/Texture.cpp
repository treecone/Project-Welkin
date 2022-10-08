#include "Texture.h"

Texture::Texture(std::string PATH, VulkanCore* vCore, short textureSpot): vCore{vCore}, textureSpot{textureSpot}
{
	pixels = stbi_load(PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	imageSize = texWidth * texHeight * 4;

	if (!pixels)
	{
		throw std::runtime_error("failed to load" + PATH + " texture image!");
	}

	if (imageSize <= 0)
	{
		throw std::runtime_error("Image size can't be smaller then 0!");
	}

	Helper::Cout("Creating buffers, images, and views for: " + PATH);

	CreateBuffers();
	textureImageView = vCore->CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
}

Texture::~Texture()
{
	vkDestroyImageView(*vCore->GetLogicalDevice(), textureImageView, nullptr);
	vkDestroyImage(*vCore->GetLogicalDevice(), textureImage, nullptr);
	vkFreeMemory(*vCore->GetLogicalDevice(), textureImageMemory, nullptr);
}

void Texture::CreateBuffers()
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
	vCore->TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	vCore->CopyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	vCore->TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(*device, stagingBuffer, nullptr);
	vkFreeMemory(*device, stagingBufferMemory, nullptr);
}

//TODO All of the helper functions that submit commands so far have been set up to execute synchronously by waiting for the queue to become idle. For practical applications it is recommended to combine these operations in a single command buffer and execute them asynchronously for higher throughput, especially the transitions and copy in the createTextureImage function. Try to experiment with this by creating a setupCommandBuffer that the helper functions record commands into, and add a flushSetupCommands to execute the commands that have been recorded so far. It's best to do this after the texture mapping works to check if the texture resources are still set up correctly.