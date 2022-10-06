#pragma once
#include <vulkan/vulkan.h>
#include "Helper.h"
#include "Texture.h"
#include <string>
#include <glm/glm.hpp>
#include "VulkanCore.h"

class Material
{
public:
	Material(Texture* color, std::string materialName, VulkanCore* vCore, glm::vec2 uvScale, unsigned short textureStart, unsigned char textureLength);
	std::string GetMaterialName() { return this->materialName; };
	VkSampler* GetSampler() { return &this->textureSampler; };
	virtual ~Material();
protected:
	void CreateTextureSampler();

	VulkanCore* vCore;
	VkSampler textureSampler;


	//Perameters
	std::string materialName;
	glm::vec2 uvScale;

	//Images
	unsigned short textureStart;
	unsigned char numberOfTextures;
	Texture* tex_Color;

};