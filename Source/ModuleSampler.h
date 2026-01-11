#pragma once
#include "Module.h"

class ModuleSampler : public Module
{
	ComPtr<ID3D12DescriptorHeap> descriptorHeap;
	unsigned descriptorSize;

public:
	ModuleSampler();
	~ModuleSampler();

	bool init() override;
	bool createDefaultSamplers();

	ID3D12DescriptorHeap* getHeap() { return this->descriptorHeap.Get(); }

	D3D12_CPU_DESCRIPTOR_HANDLE getCPUHandle(unsigned index) const;
	D3D12_GPU_DESCRIPTOR_HANDLE getGPUHandle(unsigned index) const;
};