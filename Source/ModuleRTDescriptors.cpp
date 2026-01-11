#include "Globals.h"
#include "ModuleRTDescriptors.h"
#include "Application.h"
#include "ModuleD3D12.h"

ModuleRTDescriptors::ModuleRTDescriptors() {}

ModuleRTDescriptors::~ModuleRTDescriptors() {}

bool ModuleRTDescriptors::init()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NumDescriptors = capacity;

	app->getD3D12()->getDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap));
	descriptorSize = app->getD3D12()->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	return true;
}

unsigned ModuleRTDescriptors::allocateDescriptor()
{
	unsigned index = 0;
	if (nextFreeSlotIndex < capacity)
		index = nextFreeSlotIndex++;
	return index;
}

unsigned ModuleRTDescriptors::createRTV(ID3D12Resource* resource)
{
	unsigned index = allocateDescriptor();

	D3D12_CPU_DESCRIPTOR_HANDLE handle = getCPUHandle(index);
	app->getD3D12()->getDevice()->CreateRenderTargetView(resource, nullptr, handle);

	return index;
}

void ModuleRTDescriptors::reset()
{
	nextFreeSlotIndex = 0;
}

D3D12_CPU_DESCRIPTOR_HANDLE ModuleRTDescriptors::getCPUHandle(unsigned index) const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeap->GetCPUDescriptorHandleForHeapStart(), index, app->getD3D12()->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
}