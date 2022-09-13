#include "Camera.h"

Camera::Camera(float moveSpeed, float mouseLookSpeed, float aspectRatio, float nearPlane, float farPlane):
	movementSpeed{ moveSpeed }, mouseLookSpeed{ mouseLookSpeed }, nearPlane{ nearPlane }, farPlane { farPlane}
{
	transform = Transform();
	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);
	Helper::Cout("Created the main camera!");
}

Camera::~Camera()
{

}

void Camera::Update(float dt)
{

}

void Camera::UpdateViewMatrix()
{
	viewMatrix = transform.GetWorldMatrix();
}

void Camera::UpdateViewMatrix(Transform* objToLookAt)
{
	viewMatrix = glm::lookAt(transform.GetPosition(), objToLookAt->GetPosition(), glm::vec3(0, 1, 0));
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	projMatrix = glm::perspective(FOV, aspectRatio, nearPlane, farPlane);
}
