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
	Material(Texture* tex_Color, std::string materialName, VulkanCore* vCore, Texture* tex_Roughness, Texture* tex_AO, Texture* tex_Depth, Texture* tex_Normal, glm::vec2 uvScale);
	std::string GetMaterialName() { return this->materialName; };
	Texture* GetTexture() { return this->tex_Color; };
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
	Texture* tex_Color;
	Texture* tex_Roughness;
	Texture* tex_AO;
	Texture* tex_Depth;
	Texture* tex_Normal;

};