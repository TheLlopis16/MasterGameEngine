#include "Globals.h"
#include "ModuleRingBuffer.h"
#include "Application.h"
#include "ModuleD3D12.h"

ModuleRingBuffer::ModuleRingBuffer() {}

ModuleRingBuffer::~ModuleRingBuffer() {}

bool ModuleRingBuffer::init()
{
	//4 mb
	setTotalSize(4 * 1024 * 1024);

	ModuleD3D12* d3d12 = app->getD3D12();

	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(totalSize);
	CD3DX12_HEAP_PROPERTIES props(D3D12_HEAP_TYPE_UPLOAD);
	d3d12->getDevice()->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));

	CD3DX12_RANGE readRange(0, 0);
	uploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&cpuPtr));

	gpuBase = uploadBuffer->GetGPUVirtualAddress();
	return true;
}

void ModuleRingBuffer::preRender()
{
	ModuleD3D12* d3d12 = app->getD3D12();
	unsigned idx = d3d12->getCurrentBackBufferIndex();

	size_t toFree = allocatedPerFrame[idx];
	allocatedPerFrame[idx] = 0;
	tail += toFree;

	if (tail >= totalSize)
		tail -= totalSize;
}

RingAlloc ModuleRingBuffer::allocBuffer(size_t requestedSize)
{
	ModuleD3D12* d3d12 = app->getD3D12();
	unsigned idx = d3d12->getCurrentBackBufferIndex();

	RingAlloc obj;
	size_t size = alignUp(requestedSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	bool success = false;

	if (tail > head)
	{
		if (size <= (tail - head))
		{
			obj.cpu = cpuPtr + head;
			obj.gpu = gpuBase + head;
			obj.size = size;

			head += size;
			allocatedPerFrame[idx] += size;
			success = true;
		}
	}
	else if (tail < head)
	{
		if (size <= (totalSize - head))
		{
			obj.cpu = cpuPtr + head;
			obj.gpu = gpuBase + head;
			obj.size = size;

			head += size;
			allocatedPerFrame[idx] += size;
			success = true;
		}
		else
		{
			head = 0;
			if (size <= totalSize)
			{
				obj.cpu = cpuPtr + head;
				obj.gpu = gpuBase + head;
				obj.size = size;

				head += size;
				allocatedPerFrame[idx] += size;
				success = true;
			}
		}
	}
	else if (tail == head)
	{
		if (size <= (totalSize - head))
		{
			obj.cpu = cpuPtr + head;
			obj.gpu = gpuBase + head;
			obj.size = size;

			head += size;
			allocatedPerFrame[idx] += size;
			success = true;
		}
	}

	if (!success)
		return { nullptr, 0, 0 };

	return obj;

}

void ModuleRingBuffer::setTotalSize(size_t s)
{
	totalSize = alignUp(s, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
}