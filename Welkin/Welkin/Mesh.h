#pragma once
#include "Mesh.h"
#include "Vertex.h"
#include <unordered_map>
#include <vulkan/vulkan.h>
#include <glm/gtx/hash.hpp>

using namespace std;

class Mesh
{
public:
	Mesh(string MODEL_PATH, VkDevice* device);
private:
	vector<Vertex> vertices;
	vector<uint32_t> indices;

	void LoadModel(string MODEL_PATH);
	void CreateBuffers(VkDevice* device);
	//http://foundationsofgameenginedev.com/FGED2-sample.pdf
	//void CalculateTangents();
};

namespace std
{
	template<> struct hash<Vertex>
	{
		size_t operator()(Vertex const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.UV) << 1);
		}
	};
}

namespace MathF
{
	float Q_rsqrt(float number)
	{
		//https://en.wikipedia.org/wiki/Fast_inverse_square_root
		long i;
		float x2, y;
		const float threehalfs = 1.5F;

		x2 = number * 0.5F;
		y = number;
		i = *(long*)&y;
		i = 0x5f3759df - (i >> 1);
		y = *(float*)&i;
		y = y * (threehalfs - (x2 * y * y));

		#ifndef Q3_VM
		#ifdef __linux__
				assert(!isnan(y));
		#endif
		#endif
		return y;
	}
}