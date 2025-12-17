#pragma once

#include "Module.h"

class ModuleShaderDescriptors : public Module
{
	ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	UINT descriptorSize = 0;
	UINT capacity = 32768;
	UINT nextFreeSlot = 0;

public:
	ModuleShaderDescriptors();
	~ModuleShaderDescriptors();

	bool init() override;

	UINT createSRV(ID3D12Resource* resource);

	UINT allocateDescriptor();

	D3D12_CPU_DESCRIPTOR_HANDLE getCPUHandle(UINT index) const;
	D3D12_GPU_DESCRIPTOR_HANDLE getGPUHandle(UINT index) const;

	void reset();

	ID3D12DescriptorHeap* getHeap() const { return descriptorHeap.Get(); }
	UINT getCapacity() const { return capacity; }
	UINT getNextFreeSlot() const { return nextFreeSlot; }

};
