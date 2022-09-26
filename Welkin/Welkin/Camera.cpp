#include "Camera.h"

Camera::Camera(float moveSpeed, float mouseLookSpeed, float aspectRatio, float nearPlane, float farPlane):
	movementSpeed{ moveSpeed }, mouseLookSpeed{ mouseLookSpeed }, nearPlane{ nearPlane }, farPlane { farPlane}, aspectRatio {aspectRatio}
{
	transform = Transform();
	UpdateViewMatrix();
	UpdateProjectionMatrix();
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

void Camera::UpdateProjectionMatrix()
{
	projMatrix = glm::perspective(FOV, aspectRatio, nearPlane, farPlane);
	//GLM was for OpenGL, where the y coords of the clip coords are flipped
	projMatrix[1][1] *= -1;
}
