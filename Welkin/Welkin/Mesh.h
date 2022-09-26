#pragma once
#include "Mesh.h"
#include "Vertex.h"
#include <unordered_map>
#include "Helper.h"
#include "VulkanCore.h"
#include <vulkan/vulkan.h>
#include <glm/gtx/hash.hpp>

using namespace std;

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
	vector<uint16_t> indices;

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