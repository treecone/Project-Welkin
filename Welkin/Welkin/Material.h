#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "VulkanCore.h"
#include "FileManager.h"
#include "Helper.h"
class Material
{
public:
	Material(Texture* color, string materialName, VkDevice* device);
	virtual void CreateBuffers(VkDevice* device);
	virtual ~Material();
private:
	Texture* tex_Color;
	string materialName;
};