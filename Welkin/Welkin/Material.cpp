#include "Material.h"

Material::Material(Texture* color, std::string materialName, VkDevice* device)
	: tex_Color {color}, materialName {materialName}
{
	CreateBuffers(device);
}

void Material::CreateBuffers(VkDevice* device)
{
	Helper::Cout("-- Creating Buffer for " + materialName);
}

Material::~Material()
{

}