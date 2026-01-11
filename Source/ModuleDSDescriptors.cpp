#include "Globals.h"
#include "ModuleDSDescriptors.h"
#include "Application.h"
#include "ModuleD3D12.h"

ModuleDSDescriptors::ModuleDSDescriptors() {}

ModuleDSDescriptors::~ModuleDSDescriptors() {}

bool ModuleDSDescriptors::init()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NumDescriptors = capacity;

	app->getD3D12()->getDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap));
	descriptorSize = app->getD3D12()->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	return true;
}

unsigned ModuleDSDescriptors::allocateDescriptor()
{
	unsigned index = 0;
	if (nextFreeSlotIndex < capacity)
		index = nextFreeSlotIndex++;
	return index;
}

unsigned ModuleDSDescriptors::createDSV(ID3D12Resource* resource)
{
	unsigned index = allocateDescriptor();

	D3D12_CPU_DESCRIPTOR_HANDLE handle = getCPUHandle(index);
	app->getD3D12()->getDevice()->CreateDepthStencilView(resource, nullptr, handle);

	return index;
}

void ModuleDSDescriptors::reset()
{
	nextFreeSlotIndex = 0;
}

D3D12_CPU_DESCRIPTOR_HANDLE ModuleDSDescriptors::getCPUHandle(unsigned index) const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeap->GetCPUDescriptorHandleForHeapStart(), index, app->getD3D12()->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV));
}