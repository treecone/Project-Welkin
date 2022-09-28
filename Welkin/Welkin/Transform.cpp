#include "Transform.h"

Transform::Transform()
{
	//Start matrices as identity
	worldMatrix = glm::mat4(1);
	worldInverseTransposeMatrix = glm::mat4(1);

	position = vec3(0, 0, 0);
	eulerAngles = vec3(0, 0, 0);
	scale = vec3(1, 1, 1);

	matricesDirty = false;

	parent = nullptr;
}

void Transform::MoveAbsolute(float x, float y, float z)
{
	position += vec3(x, y, z);
	matricesDirty = true;
}

void Transform::MoveRelative(float x, float y, float z)
{
	throw "Move Relative not implemented yet!";
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
	eulerAngles += vec3(pitch, yaw, roll);
	matricesDirty = true;

}

void Transform::Scale(float x, float y, float z)
{
	scale *= vec3(x, y, z);
	matricesDirty = true;

}

void Transform::SetPosition(float x, float y, float z)
{
	position = vec3(x, y, z);
	matricesDirty = true;
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	eulerAngles = vec3(pitch, yaw, roll);
	matricesDirty = true;
}

void Transform::SetScale(float x, float y, float z)
{
	scale = vec3(x, y, z);
	matricesDirty = true;
}

void Transform::SetTransformsFromMatrix(mat4 worldMatrix)
{
	//Decompose Matrix
	glm::mat4 transformation; // your transformation matrix.
	glm::vec3 scale;
	glm::quat rotation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(transformation, scale, rotation, translation, skew, perspective);
	rotation = glm::conjugate(rotation);

	position = translation;
	eulerAngles = QuaterionToEuler(rotation);
	this->scale = scale;

	matricesDirty = true;
}

vec3 Transform::GetPosition() { return position; }

vec3 Transform::GetPitchYawRoll() { return eulerAngles; }

vec3 Transform::GetScale() { return scale; }

mat4 Transform::GetWorldMatrix() { return worldMatrix; }

mat4 Transform::GetWorldInverseTransposeMatrix() { return worldInverseTransposeMatrix; }

//Hiarchy

void Transform::UpdateMatrices()
{
	if (matricesDirty)
	{
		mat4 worldMatrix = mat4(1.0f);

		mat4 translationMatrix = translate(mat4(1.0f), position);
		mat4 scaleMatrix = glm::scale(mat4(1.0f), scale);
		mat4 rotationMatrix = glm::eulerAngleXYZ(eulerAngles.x, eulerAngles.y, eulerAngles.z);

		//TODO right order?
		//worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;
		worldMatrix = translationMatrix * rotationMatrix * scaleMatrix;

		if (parent)
		{
			worldMatrix *= parent->GetWorldMatrix();
		}

		this->worldMatrix = worldMatrix;
		this->worldInverseTransposeMatrix = glm::inverse(glm::transpose(worldMatrix));

		matricesDirty = false;

		MarkChildTransformsDirty();
	}
}

void Transform::MarkChildTransformsDirty()
{
	for (auto child : children)
	{
		child->matricesDirty = true;
		child->MarkChildTransformsDirty();
	}
}

vec3 Transform::QuaterionToEuler(glm::quat quaterion)
{
	return glm::eulerAngles(quaterion);
}