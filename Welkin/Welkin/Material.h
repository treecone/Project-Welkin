#pragma once
#include <vulkan/vulkan.h>
#include "Helper.h"
#include "Texture.h"
#include <string>

class Material
{
public:
	Material(Texture* color, std::string materialName, VkDevice* device);
	virtual void CreateBuffers(VkDevice* device);
	std::string GetMaterialName() { return this->materialName; };
	virtual ~Material();
protected:
	Texture* tex_Color;
	std::string materialName;
};