#pragma once
#include "Material.h"

class PBRMaterial : public Material
{
public:
	PBRMaterial(Texture* tex_Color, std::string materialName, VkDevice* device,  Texture* tex_Roughness, Texture* tex_AO, Texture* tex_Depth, Texture* tex_Normal);
	void CreateBuffers(VkDevice* device) override;
	~PBRMaterial() override;
private:
	Texture* tex_Roughness;
	Texture* tex_AO;
	Texture* tex_Depth;
	Texture* tex_Normal;

};
