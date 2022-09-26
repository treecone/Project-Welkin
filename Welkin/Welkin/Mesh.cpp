#include "Mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

Mesh::Mesh(string MODEL_PATH, VulkanCore* vCore): vCore(vCore)
{
	LoadModel(MODEL_PATH);
	CreateVertexBuffer();
	CreateIndexBuffer();
}

VkBuffer* Mesh::GetVertexBuffer()
{
	return &vertexBuffer;
}

VkBuffer* Mesh::GetIndexBuffer()
{
	return &indexBuffer;
}

uint32_t Mesh::GetVerticesSize()
{
	return vertices.size();
}

uint32_t Mesh::GetIndeicesSize()
{
	return this->indices.size();
}

Mesh::~Mesh()
{
	vkDestroyBuffer(*vCore->GetLogicalDevice(), vertexBuffer, nullptr);
	vkFreeMemory(*vCore->GetLogicalDevice(), vertexBufferMemory, nullptr);

	vkDestroyBuffer(*vCore->GetLogicalDevice(), indexBuffer, nullptr);
	vkFreeMemory(*vCore->GetLogicalDevice(), indexBufferMemory, nullptr);
}

void Mesh::LoadModel(std::string MODEL_PATH)
{

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, MODEL_PATH.c_str()))
	{
		throw std::runtime_error(err);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes) 
	{
		for (const auto& index : shape.mesh.indices) 
		{
			Vertex vertex{};

			if(index.vertex_index >= 0)
			vertex.position = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			//0 is the top, not bottom, flip
			vertex.UV = 
			{
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			if (index.normal_index >= 0)
			vertex.normal = {

				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
			};

			if (uniqueVertices.count(vertex) == 0) 
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}

		//For each triangle, calculate tangents
		//CalculateTangents();
	}

	Helper::Cout("Loaded Mesh: [" + MODEL_PATH + "]");

}

void Mesh::CreateVertexBuffer()
{
	VkDevice device = *vCore->GetLogicalDevice();
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	vCore->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
		stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	vCore->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | 
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
		vertexBuffer, vertexBufferMemory);

	vCore->CopyBuffer(stagingBuffer, vertexBuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

	Helper::Cout("- Vertex Buffer Memory Bound and Created");

}

void Mesh::CreateIndexBuffer()
{
	VkDevice device = *vCore->GetLogicalDevice();
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	vCore->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	vCore->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		indexBuffer, indexBufferMemory);

	vCore->CopyBuffer(stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

	Helper::Cout("- Index Buffer Memory Bound and Created");
}
