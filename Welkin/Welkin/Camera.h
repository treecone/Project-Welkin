#pragma once
#include "Transform.h"
#include <glm/glm.hpp>

class Camera
{
	Camera(Transform transform, float moveSpeed, float mouseLookSpeed, float aspectRatio);
	~Camera();

	void Update(float dt);
	void UpdateViewMatrix();
	void UpdateProjectionMatrix(float aspectRatio);

	glm::mat4 GetView() { return viewMatrix; }
	glm::mat4 GetProjection() { return projMatrix; }

	Transform* GetTransform() { return &this->transform; }

private:

	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;

	Transform transform;

	float movementSpeed;
	float mouseLookSpeed;
};

