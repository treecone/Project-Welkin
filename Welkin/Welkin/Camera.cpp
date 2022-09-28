#include "Camera.h"
#include "Input.h"

Camera::Camera(float moveSpeed, float mouseLookSpeed, float aspectRatio, float nearPlane, float farPlane):
	movementSpeed{ moveSpeed }, mouseLookSpeed{ mouseLookSpeed }, nearPlane{ nearPlane }, farPlane { farPlane}, aspectRatio {aspectRatio}
{
	transform = Transform();
	UpdateViewMatrix();
	SetFOV(90.0f);
	UpdateProjectionMatrix();
	Helper::Cout("Created the main camera!");
}

Camera::~Camera()
{

}

void Camera::Update(float dt)
{
	float speed = 0.1f;
	if (Input::GetInstance().KeyDown(GLFW_KEY_W))
	{
		this->transform.MoveAbsolute(speed * dt, 0, 0);
		glm::vec3 temp = transform.GetPosition();
		Helper::Cout("Camera Position: " + std::to_string(temp.x) + "," + std::to_string(temp.y) + "," + std::to_string(temp.z));
	}
	if (Input::GetInstance().KeyDown(GLFW_KEY_S))
	{
		this->transform.MoveAbsolute(-speed * dt, 0, 0);
		glm::vec3 temp = transform.GetPosition();
		Helper::Cout("Camera Position: " + std::to_string(temp.x) + "," + std::to_string(temp.y) + "," + std::to_string(temp.z));
	}
	if (Input::GetInstance().KeyDown(GLFW_KEY_A))
	{
		this->transform.MoveAbsolute(0, -speed * dt, 0);
		glm::vec3 temp = transform.GetPosition();
		Helper::Cout("Camera Position: " + std::to_string(temp.x) + "," + std::to_string(temp.y) + "," + std::to_string(temp.z));
	}
	if (Input::GetInstance().KeyDown(GLFW_KEY_D))
	{
		this->transform.MoveAbsolute(0, speed * dt, 0);
		glm::vec3 temp = transform.GetPosition();
		Helper::Cout("Camera Position: " + std::to_string(temp.x) + "," + std::to_string(temp.y) + "," + std::to_string(temp.z));

	}

	if (Input::GetInstance().KeyDown(GLFW_KEY_F))
	{
		FOV += 0.1f;
		Helper::Cout(std::to_string(FOV));
	}
	else if (Input::GetInstance().KeyDown(GLFW_KEY_V))
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
