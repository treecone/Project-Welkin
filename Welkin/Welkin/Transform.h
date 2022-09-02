#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <vector>

using namespace glm;


class Transform
{
public:
	Transform();

	void MoveAbsolute(float x, float y, float z);
	void MoveRelative(float x, float y, float z);
	void Rotate(float pitch, float yaw, float roll);
	void Scale(float x, float y, float z);

	void SetPosition(float x, float y, float z);
	void SetRotation(float pitch, float yaw, float roll);
	void SetScale(float x, float y, float z);

	void SetTransformsFromMatrix(mat4 worldMatrix);

	vec3 GetPosition();
	vec3 GetPitchYawRoll();
	vec3 GetScale();

	mat4 GetWorldMatrix();
	mat4 GetWorldInverseTransposeMatrix();

	void AddChild(Transform* child, bool makeChildRelative);
	void RemoveChild(Transform* child, bool applyParentTransform);
	void SetParent(Transform* newParent, bool makeChildRelative);
	Transform* GetParent();
	Transform* GetChild(unsigned int index);
	int IndexOfChild(Transform* child);
	unsigned int ChildCount();

private:
	//Hierarchy
	Transform* parent;
	std::vector<Transform*> children;

	//Transform Data
	vec3 position;
	vec3 eulerAngles;
	vec3 scale;

	//World matrix and such
	bool matricesDirty;
	mat4 worldMatrix;
	mat4 worldInverseTransposeMatrix;

	//Update helper funtions
	void UpdateMatrices();
	void MarkChildTransformsDirty();

	//Helper for conversions
	vec3 QuaterionToEuler(quat quaterion);



};

