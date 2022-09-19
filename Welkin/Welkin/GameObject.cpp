#include "GameObject.h"

GameObject::GameObject(string objectName, Mesh* mesh, Material* material):
	name{ objectName }, mesh{mesh}, material {material}
{
	if (objectName != "" && mesh != nullptr && material != nullptr)
	{
		Helper::Cout("[" + objectName + "] Gameobject Created!");
	}
	else
	{
		Helper::Warning("[" + objectName + "] Gameobject is incomplete!");
	}
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
