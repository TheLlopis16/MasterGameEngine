#include "Globals.h"

#include "ModuleShaderDescriptors.h"
#include "Application.h"
#include "ModuleD3D12.h"

ModuleShaderDescriptors::ModuleShaderDescriptors() {}

ModuleShaderDescriptors::~ModuleShaderDescriptors() {}

bool ModuleShaderDescriptors::init()
{

    ModuleD3D12* d3d12 = app->getD3D12();
    ID3D12Device5* device = d3d12->getDevice();

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = capacity;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap));
    descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    nextFreeSlot = 0;

    return true;
}

UINT ModuleShaderDescriptors::allocateDescriptor()
{
    assert(nextFreeSlot < capacity);
    return nextFreeSlot++;
}


UINT ModuleShaderDescriptors::createSRV(ID3D12Resource* resource)
{
    ModuleD3D12* d3d12 = app->getD3D12();
    ID3D12Device5* device = d3d12->getDevice();

    UINT index = allocateDescriptor();
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = getCPUHandle(index);

    device->CreateShaderResourceView(resource, nullptr, cpuHandle);

    return index;
}

D3D12_CPU_DESCRIPTOR_HANDLE ModuleShaderDescriptors::getCPUHandle(UINT index) const
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<SIZE_T>(index) * descriptorSize;
    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE ModuleShaderDescriptors::getGPUHandle(UINT index) const
{
    assert(index < capacity && "getGPUHandle index out of range");

    D3D12_GPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<UINT64>(index) * descriptorSize;
    return handle;
}

void ModuleShaderDescriptors::reset()
{
    nextFreeSlot = 0;
}