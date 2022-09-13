#pragma once
#include "Transform.h"
#include "Helper.h"

class Camera
{
public:
	Camera(float moveSpeed, float mouseLookSpeed, float aspectRatio, float nearPlane, float farPlane);
	~Camera();

	void SetFOV(float fov) { FOV = fov; };
	void Update(float dt);
	void UpdateViewMatrix();
	void UpdateViewMatrix(Transform* objToLookAt);
	void UpdateProjectionMatrix(float aspectRatio);

	glm::mat4 GetView() { return viewMatrix; }
	glm::mat4 GetProjection() { return projMatrix; }

	Transform* GetTransform() { return &this->transform; }

private:

	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;
	float FOV;
	float nearPlane;
	float farPlane;

	Transform transform;

	float movementSpeed;
	float mouseLookSpeed;
};

