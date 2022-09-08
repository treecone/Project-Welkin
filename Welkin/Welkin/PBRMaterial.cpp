#include "PBRMaterial.h"

PBRMaterial::PBRMaterial(Texture* tex_Color, string materialName, VkDevice* device,  Texture* tex_Roughness, Texture* tex_AO, Texture* tex_Depth, Texture* tex_Normal)
	: Material { tex_Color, materialName, device },
	tex_Roughness {tex_Roughness}, tex_AO {tex_AO}, tex_Depth {tex_Depth}, tex_Normal {tex_Normal}
{
	
}

void CreateBuffers(VkDevice* device) 
{
	Helper::Cout("Creating Buffer for Material " + materialName);
}