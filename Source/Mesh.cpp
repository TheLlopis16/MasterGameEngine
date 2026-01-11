#include "Globals.h"
#include "Mesh.h"
#include "utils_gltf.h"
#include "Application.h"
#include "ModuleResources.h"

Mesh::Mesh() {}

Mesh::~Mesh() {}

void Mesh::load(const tinygltf::Model& model, const tinygltf::Mesh& mesh, const tinygltf::Primitive& primitive)
{
	materialIndex = primitive.material;

	const auto& pos = primitive.attributes.find("POSITION");

	if (pos != primitive.attributes.end())
	{
		numVertices = uint32_t(model.accessors[pos->second].count);
		vertices = std::make_unique<Vertex[]>(numVertices);
		uint8_t* vertexData = (uint8_t*)vertices.get();

		loadAccessorData(vertexData + offsetof(Vertex, position), sizeof(Vector3), sizeof(Vertex),
			numVertices, model, pos->second);
		loadAccessorData(vertexData + offsetof(Vertex, texCoord0), sizeof(Vector2), sizeof(Vertex),
			numVertices, model, primitive.attributes, "TEXCOORD_0");
		loadAccessorData(vertexData + offsetof(Vertex, normal), sizeof(Vector3), sizeof(Vertex),
			numVertices, model, primitive.attributes, "NORMAL");

		vertexBuffer = app->getResources()->createDefaultBuffer(numVertices * sizeof(Vertex), vertices.get());

		vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vertexBufferView.SizeInBytes = numVertices * sizeof(Vertex);
		vertexBufferView.StrideInBytes = sizeof(Vertex);

	}

	if (primitive.indices >= 0)
	{
		const tinygltf::Accessor& indicesAccessor = model.accessors[primitive.indices];

		if (indicesAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT ||
			indicesAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT ||
			indicesAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
		{
			uint32_t indexElementSize = tinygltf::GetComponentSizeInBytes(indicesAccessor.componentType);
			numIndices = uint32_t(indicesAccessor.count);

			indices = std::make_unique<uint8_t[]>(numIndices * indexElementSize);
			loadAccessorData(indices.get(), indexElementSize, indexElementSize, numIndices, model, primitive.indices);

			indexBuffer = app->getResources()->createDefaultBuffer(numIndices * indexElementSize, indices.get());

			indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
			indexBufferView.SizeInBytes = numIndices * indexElementSize;
			
			if (indicesAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
				indexBufferView.Format = DXGI_FORMAT_R8_UINT;
			else if (indicesAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
				indexBufferView.Format = DXGI_FORMAT_R16_UINT;
			else if (indicesAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
				indexBufferView.Format = DXGI_FORMAT_R32_UINT;

		}
	}
}