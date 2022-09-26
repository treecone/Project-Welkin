#include "Material.h"

Material::Material(Texture* color, std::string materialName, VkDevice* device, glm::vec2 uvScale)
	: tex_Color {color}, materialName {materialName}, uvScale {uvScale}
{
	CreateBuffers(device);
}

void Material::CreateBuffers(VkDevice* device)
{
	Helper::Cout("- Creating Buffer for " + materialName);
}

Material::~Material()
{

}