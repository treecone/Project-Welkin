#pragma once
#include "Mesh.h"
#include "Camera.h"
#include "Transform.h"
#include "Material.h"

class GameObject
{
public:
	GameObject(string name, Mesh* mesh, Material* material);

	Mesh* GetMesh();
	Material* GetMaterial();
	Transform* GetTransform();

	void SetMaterial(Material* material);

	string name;

	bool operator < (const GameObject& str) const
	{
		return (this->material->GetMaterialName() < str.material->GetMaterialName());
	}

private:

	Mesh* mesh;
	Material* material;
	Transform transform;
};