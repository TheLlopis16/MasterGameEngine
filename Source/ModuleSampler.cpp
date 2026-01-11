#include "Globals.h"
#include "ModuleSampler.h"
#include "Application.h"
#include "ModuleD3D12.h"

ModuleSampler::ModuleSampler() {}

ModuleSampler::~ModuleSampler() {}

bool ModuleSampler::init()
{
	ModuleD3D12* d3d12 = app->getD3D12();
	descriptorSize = d3d12->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = 4;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	bool succeed = SUCCEEDED(d3d12->getDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));
	succeed = succeed && SUCCEEDED(createDefaultSamplers());
	return succeed;
}

bool ModuleSampler::createDefaultSamplers()
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_SAMPLER_DESC descs[4] = {
		// Linear Wrap
		{
			D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			0.0f,
			1,
			D3D12_COMPARISON_FUNC_NONE,
			{ 0.0f, 0.0f, 0.0f, 0.0f },
			0.0f, D3D12_FLOAT32_MAX
		},

		// Point Wrap
		{
			D3D12_FILTER_MIN_MAG_MIP_POINT,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			0.0f,
			1,
			D3D12_COMPARISON_FUNC_NONE,
			{ 0.0f, 0.0f, 0.0f, 0.0f },
			0.0f, D3D12_FLOAT32_MAX
		},

		// Linear Clamp
		{
			D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			0.0f,
			1,
			D3D12_COMPARISON_FUNC_NONE,
			{ 0.0f, 0.0f, 0.0f, 0.0f },
			0.0f, D3D12_FLOAT32_MAX
		},

		// Point Clamp
		{
			D3D12_FILTER_MIN_MAG_MIP_POINT,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			0.0f,
			1,
			D3D12_COMPARISON_FUNC_NONE,
			{ 0.0f, 0.0f, 0.0f, 0.0f },
			0.0f, D3D12_FLOAT32_MAX
		}
	};

	ModuleD3D12* d3d12 = app->getD3D12();

	for (int i = 0; i < std::size(descs); ++i)
	{
		d3d12->getDevice()->CreateSampler(&descs[i], handle);
		handle.ptr += descriptorSize;
	}

	return true;
}

D3D12_CPU_DESCRIPTOR_HANDLE ModuleSampler::getCPUHandle(unsigned index) const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeap->GetCPUDescriptorHandleForHeapStart(), index, descriptorSize);
}

D3D12_GPU_DESCRIPTOR_HANDLE ModuleSampler::getGPUHandle(unsigned index) const
{
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetGPUDescriptorHandleForHeapStart(), index, descriptorSize);
}