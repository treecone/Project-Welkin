#pragma once
#include <vulkan/vulkan.h>
#include "Helper.h"
#include "Texture.h"
#include <string>
#include <glm/glm.hpp>

class Material
{
public:
	Material(Texture* color, std::string materialName, VkDevice* device, glm::vec2 uvScale);
	virtual void CreateBuffers(VkDevice* device);
	std::string GetMaterialName() { return this->materialName; };
	virtual ~Material();
protected:

	//Perameters
	std::string materialName;
	glm::vec2 uvScale;
	//Images
	Texture* tex_Color;

};