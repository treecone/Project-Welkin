#include "GameObject.h"

GameObject::GameObject(Mesh* mesh, Material* material)
{

}

Mesh* GameObject::GetMesh()
{
	return this->mesh;
}

Material* GameObject::GetMaterial()
{
	return this->material;
}

Transform* GameObject::GetTransform()
{
	return &this->transform;
}

void GameObject::SetMaterial(Material* material)
{
	this->material = material;
}

void GameObject::Draw(VkDevice* device, Camera* camera)
{
	
}
