#pragma once
#include "Transform.h"
#include "Helper.h"
#include "Input.h"

class Camera
{
public:
	Camera(float moveSpeed, float mouseLookSpeed, float fov, float aspectRatio, float nearPlane, float farPlane, Input* input);
	~Camera();

	void SetFOV(float fov) { FOV = fov; };
	void Update(float dt);
	void UpdateViewMatrix();
	void UpdateViewMatrix(Transform* objToLookAt);
	void UpdateProjectionMatrix();

	glm::mat4 GetView() { return viewMatrix; }
	glm::mat4 GetProjection() { return projMatrix; }

	Transform* GetTransform() { return &this->transform; }

private:
	Input* input;

	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;
	float FOV;
	float nearPlane;
	float farPlane;
	float aspectRatio;

	Transform transform;

	float movementSpeed;
	float mouseLookSpeed;
};

