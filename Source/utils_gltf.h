#pragma once

#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE 
#include "tiny_gltf.h"

inline bool loadAccessorData(uint8_t* data, size_t elemSize, size_t stride, size_t elemCount, const tinygltf::Model& model, int accessorIndex)
{
	const tinygltf::Accessor& accessor = model.accessors[accessorIndex];

	if (elemCount != accessor.count)
		return false;

	size_t bStride = tinygltf::GetComponentSizeInBytes(accessor.componentType) * tinygltf::GetNumComponentsInType(accessor.type);

	if (bStride != elemSize)
		return false;

	const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
	const tinygltf::Buffer& buffer = model.buffers[view.buffer];

	size_t startAddress = (size_t)view.byteOffset + (size_t)accessor.byteOffset;

	const uint8_t* sourceData = buffer.data.data() + startAddress;

	size_t sourceStride = (size_t)view.byteStride;
	if (view.byteStride == 0)
		sourceStride = bStride;

	for (size_t i = 0; i < elemCount; ++i)
	{
		std::memcpy(data, sourceData, elemSize);
		data += stride;
		sourceData += sourceStride;
	}

	return true;
}

inline bool loadAccessorData(uint8_t* data, size_t elemSize, size_t stride, size_t elemCount, const tinygltf::Model& model,
	const std::map<std::string, int>& attributes, const char* accesorName)
{
	const auto& it = attributes.find(accesorName);
	if (it != attributes.end())
	{
		return loadAccessorData(data, elemSize, stride, elemCount, model, it->second);
	}
	return false;
}