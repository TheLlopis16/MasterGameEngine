#pragma once

#include "Module.h"

class ModuleDSDescriptors : public Module
{
	ComPtr<ID3D12DescriptorHeap> descriptorHeap;
	unsigned nextFreeSlotIndex = 0;
	unsigned descriptorSize = 0;
	unsigned capacity = 16;

public:
	ModuleDSDescriptors();
	~ModuleDSDescriptors();

	bool init() override;
	unsigned allocateDescriptor();
	unsigned createDSV(ID3D12Resource* resource);
	void reset();

	ID3D12DescriptorHeap* getHeap() { return this->descriptorHeap.Get(); }

	D3D12_CPU_DESCRIPTOR_HANDLE getCPUHandle(unsigned index) const;
};