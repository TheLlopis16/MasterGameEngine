#pragma once

#include "Module.h"

#include <filesystem>
#include <vector>

namespace DirectX { class ScratchImage;  struct TexMetadata; }

class ModuleResources : public Module
{
	ComPtr<ID3D12CommandAllocator> commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> commandList;

public:

	ModuleResources();
	~ModuleResources();

	bool init() override;

	ComPtr<ID3D12Resource> createUploadBuffer(size_t size, void* cpuData);
	ComPtr<ID3D12Resource> createDefaultBuffer(size_t size, void* data);
	ComPtr<ID3D12Resource> createTextureFromFile(const std::filesystem::path& path);
};