#pragma once

#include <string>

namespace tinygltf { class Model; struct Mesh; struct Primitive; }

class Mesh
{
public:
	struct Vertex
	{
		Vector3 position = Vector3::Zero;
		Vector2 texCoord0 = Vector2::Zero;
		Vector3 normal = Vector3::Zero;
	};

private:

	std::unique_ptr<Vertex[]> vertices;
	std::unique_ptr<uint8_t[]> indices;
	int materialIndex = -1;
	uint32_t numIndices = 0;
	uint32_t numVertices = 0;

	ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	
	ComPtr<ID3D12Resource> indexBuffer;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

public:
	Mesh();
	~Mesh();

	void load(const tinygltf::Model& model, const tinygltf::Mesh& mesh, const tinygltf::Primitive& primitive);

	D3D12_VERTEX_BUFFER_VIEW* getVBV() { return &vertexBufferView; }
	D3D12_INDEX_BUFFER_VIEW* getIBV() { return &indexBufferView; }

	const int& getMatIndex() const { return materialIndex; }

	const uint32_t& getIndicesNum() const { return numIndices; }
	const uint32_t& getVerticesNum() const { return numVertices; }

	bool isIndexed() const { return numIndices > 0; }

};