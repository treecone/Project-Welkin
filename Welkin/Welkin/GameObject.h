#pragma once

#include "Mesh.h"
#include "Camera.h"
#include <vulkan/vulkan.h>
#include "Transform.h"
#include "Material.h"

class GameObject
{
public:
	GameObject(Mesh* mesh, Material* material);

	Mesh* GetMesh();
	Material* GetMaterial();
	Transform* GetTransform();

	void SetMaterial(Material* material);

	void Draw(VkDevice* device, Camera* camera);

private:

	Mesh* mesh;
	Material* material;
	Transform transform;
};

