#pragma once

#include "Module.h"

struct RingAlloc
{
	void* cpu;
	D3D12_GPU_VIRTUAL_ADDRESS gpu;
	size_t size;
};

class ModuleRingBuffer : public Module
{
	ComPtr<ID3D12Resource> uploadBuffer;
	uint8_t* cpuPtr = nullptr;
	D3D12_GPU_VIRTUAL_ADDRESS gpuBase = 0;
	size_t totalSize = 0;
	size_t head = 0;
	size_t tail = 0;
	size_t allocatedPerFrame[FRAMES_IN_FLIGHT] = { 0, 0, 0 };

public:
	ModuleRingBuffer();
	~ModuleRingBuffer();

	bool init() override;
	void preRender() override;
	RingAlloc allocBuffer(size_t requestedSize);

private:
	void setTotalSize(size_t s);
};