#include "Camera.h"

Camera::Camera(float moveSpeed, float mouseLookSpeed, float fov, float aspectRatio, float nearPlane, float farPlane, Input* input):
	movementSpeed{ moveSpeed }, mouseLookSpeed{ mouseLookSpeed }, FOV{fov}, nearPlane {nearPlane}, farPlane{ farPlane }, aspectRatio{ aspectRatio },
	input{input}
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
	float speed = 0.1f;
	if (input->KeyDown(GLFW_KEY_W))
	{
		this->transform.MoveAbsolute(0, 0, speed * dt);
		glm::vec3 temp = transform.GetPosition();
		Helper::Cout("Camera Position: " + std::to_string(temp.x) + "," + std::to_string(temp.y) + "," + std::to_string(temp.z));
	}
	if (input->KeyDown(GLFW_KEY_S))
	{
		this->transform.MoveAbsolute(0, 0, -speed * dt);
		glm::vec3 temp = transform.GetPosition();
		Helper::Cout("Camera Position: " + std::to_string(temp.x) + "," + std::to_string(temp.y) + "," + std::to_string(temp.z));
	}
	if (input->KeyDown(GLFW_KEY_A))
	{
		this->transform.MoveAbsolute(speed * dt, 0, 0);
		glm::vec3 temp = transform.GetPosition();
		Helper::Cout("Camera Position: " + std::to_string(temp.x) + "," + std::to_string(temp.y) + "," + std::to_string(temp.z));
	}
	if (input->KeyDown(GLFW_KEY_D))
	{
		this->transform.MoveAbsolute(-speed * dt,0 , 0);
		glm::vec3 temp = transform.GetPosition();
		Helper::Cout("Camera Position: " + std::to_string(temp.x) + "," + std::to_string(temp.y) + "," + std::to_string(temp.z));
	}
	if (input->KeyDown(GLFW_KEY_SPACE))
	{
		this->transform.MoveAbsolute(0, -speed * dt, 0);
		glm::vec3 temp = transform.GetPosition();
		Helper::Cout("Camera Position: " + std::to_string(temp.x) + "," + std::to_string(temp.y) + "," + std::to_string(temp.z));
	}
	if (input->KeyDown(GLFW_KEY_LEFT_CONTROL))
	{
		this->transform.MoveAbsolute(0, speed * dt, 0);
		glm::vec3 temp = transform.GetPosition();
		Helper::Cout("Camera Position: " + std::to_string(temp.x) + "," + std::to_string(temp.y) + "," + std::to_string(temp.z));
	}

	if (input->KeyDown(GLFW_KEY_F))
	{
		FOV += 0.1f;
		Helper::Cout(std::to_string(FOV));
	}
	else if (input->KeyDown(GLFW_KEY_V))
	{
		FOV -= 0.1f;
		Helper::Cout(std::to_string(FOV));
	}
}

void Camera::UpdateViewMatrix()
{
	viewMatrix = mat4(1.0f);
	transform.UpdateMatrices();
	viewMatrix = transform.GetWorldMatrix();
}

void Camera::UpdateViewMatrix(Transform* objToLookAt)
{
	viewMatrix = mat4(1.0f);
	viewMatrix = glm::lookAt(transform.GetPosition(), objToLookAt->GetPosition(), glm::vec3(0, 1, 0));
}

void Camera::UpdateProjectionMatrix()
{
	projMatrix = mat4(1.0f);
	projMatrix = glm::perspective(FOV, aspectRatio, nearPlane, farPlane);
	//GLM was for OpenGL, where the y coords of the clip coords are flipped
	projMatrix[1][1] *= -1;
}
