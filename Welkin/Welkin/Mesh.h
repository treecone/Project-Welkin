#pragma once
#include "Vertex.h"
#include <unordered_map>
#include "Helper.h"
#include <vulkan/vulkan.h>
#include <glm/gtx/hash.hpp>

using namespace std;

namespace std
{
	inline void hash_combine(std::size_t& seed) { }

	template <typename T, typename... Rest>
	inline void hash_combine(std::size_t& seed, const T& v, Rest... rest) {
		std::hash<T> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		hash_combine(seed, rest...);
	}

	template<> struct hash<Vertex>
	{
		size_t operator()(Vertex const& vertex) const
		{
			size_t seed = 0;
			hash_combine(seed, vertex.position, vertex.UV, vertex.normal);
			return seed;
		}
	};
}

class VulkanCore;

class Mesh
{
public:
	Mesh(string MODEL_PATH, VulkanCore* vCore);
	VkBuffer* GetVertexBuffer();
	VkBuffer* GetIndexBuffer();
	uint32_t GetVerticesSize();
	uint32_t GetIndeicesSize();

	~Mesh();
private:
	VulkanCore* vCore;
	vector<Vertex> vertices;
	vector<uint32_t> indices;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;


	void LoadModel(string MODEL_PATH);
	void CreateVertexBuffer();
	void CreateIndexBuffer();
	//http://foundationsofgameenginedev.com/FGED2-sample.pdf
	//void CalculateTangents();
};