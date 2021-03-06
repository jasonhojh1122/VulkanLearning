#pragma once

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <unordered_map>

#include "Vertex.h"
#include "Buffer.h"
#include "CommandPool.h"

class Model {
public:
	~Model();
	Model(LogicalDevice* device, std::string path, CommandPool* commandPool);
	Buffer* getVertexBufferRef() { return vertexBuffer; }
	Buffer* getIndexBufferRef() { return indexBuffer; }
	uint32_t getIndicesCount() { return static_cast<uint32_t>(indices.size()); }
	
private:
	void loadModel(std::string path);
	void createVertexBuffer();
	void createIndexBuffer();

	LogicalDevice* device;
	CommandPool* commandPool;

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	Buffer* vertexBuffer;
	Buffer* indexBuffer;
};

Model::~Model() {
	delete vertexBuffer;
	delete indexBuffer;
}

Model::Model(LogicalDevice* inDevice, std::string path, CommandPool* inCommandPool) {
	device = inDevice;
	commandPool = inCommandPool;
	loadModel(path);
	createVertexBuffer();
	createIndexBuffer();
}

void Model::loadModel(std::string path) {
	std::cout << "Loading model\n";
	std::string warn, err;
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
		throw std::runtime_error("Failed to load model.");

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};
	std::cout << 0 << '\n';

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex{};

			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color = { 1.0f, 1.0f, 1.0f };
			std::cout << 1 << '\n';
			vertex.normal = {
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
			};
			std::cout << 2 << '\n';
			
			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}
			std::cout << 3 << '\n';
			indices.push_back(uniqueVertices[vertex]);
			std::cout << 4 << '\n';
		}
	}
	std::cout << "Finished loading model.\n";
}

void Model::createVertexBuffer() {
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	Buffer* stagingBuffer = new Buffer(device, bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	stagingBuffer->copyDataToBuffer(vertices.data());

	vertexBuffer = new Buffer(device, bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vertexBuffer->copyBufferToBuffer(stagingBuffer, commandPool);

	delete stagingBuffer;
}

void Model::createIndexBuffer() {
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	Buffer* stagingBuffer = new Buffer(device, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	stagingBuffer->copyDataToBuffer(indices.data());

	indexBuffer = new Buffer(device, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	indexBuffer->copyBufferToBuffer(stagingBuffer, commandPool);

	delete stagingBuffer;
}