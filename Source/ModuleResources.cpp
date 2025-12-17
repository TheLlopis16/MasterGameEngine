#include "Globals.h"

#include "ModuleResources.h"
#include "Application.h"
#include "ModuleD3D12.h"

ModuleResources::ModuleResources() {}

ModuleResources::~ModuleResources() {}

bool ModuleResources::init()
{
	ModuleD3D12* d3d12 = app->getD3D12();
	ID3D12Device5* device = d3d12->getDevice();
	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&commandList));
	commandList->Reset(commandAllocator.Get(), nullptr);

	return true;
}

ComPtr<ID3D12Resource> ModuleResources::createUploadBuffer(size_t size, void* cpuData)
{
	ModuleD3D12* d3d12 = app->getD3D12();
	ID3D12Device5* device = d3d12->getDevice();
	ComPtr<ID3D12Resource> buffer;

	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size);
	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer));

	BYTE* pData = nullptr;
	CD3DX12_RANGE readRange(0, 0);
	buffer->Map(0, &readRange, reinterpret_cast<void**>(&pData));

	memcpy(pData, cpuData, size);

	buffer->Unmap(0, nullptr);

	return buffer;
}

ComPtr<ID3D12Resource> ModuleResources::createDefaultBuffer(size_t size, void* data)
{
	ModuleD3D12* d3d12 = app->getD3D12();
	ID3D12Device5* device = d3d12->getDevice();
	ID3D12CommandQueue* commandQueue = d3d12->getCommandQueue();

	ComPtr<ID3D12Resource> vertexBuffer;

	CD3DX12_HEAP_PROPERTIES defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
	device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&vertexBuffer));

	ComPtr<ID3D12Resource> stagingBuffer;

	CD3DX12_HEAP_PROPERTIES uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC stagingDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
	device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &stagingDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&stagingBuffer));

	BYTE* pData = nullptr;
	CD3DX12_RANGE readRange(0, 0);
	stagingBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pData));
	memcpy(pData, data, size);
	stagingBuffer->Unmap(0, nullptr);

	commandList->CopyBufferRegion(vertexBuffer.Get(), 0, stagingBuffer.Get(), 0, size);
	commandList->Close();

	ID3D12CommandList* commandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(UINT(std::size(commandLists)), commandLists);

	d3d12->flush();

	commandAllocator->Reset();
	commandList->Reset(commandAllocator.Get(), nullptr);

	return vertexBuffer;
}