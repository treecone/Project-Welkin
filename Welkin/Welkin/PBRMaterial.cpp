#include "PBRMaterial.h"

PBRMaterial::PBRMaterial(Texture* tex_Color, std::string materialName, VulkanCore* vCore,  Texture* tex_Roughness, Texture* tex_AO, Texture* tex_Depth, Texture* tex_Normal, glm::vec2 uvScale)
	: Material { tex_Color, materialName, vCore, uvScale },
	tex_Roughness {tex_Roughness}, tex_AO {tex_AO}, tex_Depth {tex_Depth}, tex_Normal {tex_Normal}
{
	CreateTextureSampler();
}

PBRMaterial::~PBRMaterial()
{

}