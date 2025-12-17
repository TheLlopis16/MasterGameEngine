#include "Globals.h"
#include "Exercise2.h"

#include "Application.h"
#include "ModuleD3D12.h"
#include "ModuleResources.h"
#include "ReadData.h"

#include <d3d12.h>
#include "d3dx12.h"

bool Exercise2::init()
{
	createVertexBuffer();
	bool ok = createRootSignature();
	createPSO();

	return ok;
}

void Exercise2::render()
{
	ModuleD3D12* d3d12 = app->getD3D12();
	ID3D12GraphicsCommandList* commandList = d3d12->getCommandList();

	commandList->Reset(d3d12->getCurrentCommandAllocator(), PSO.Get());
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3d12->getCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &barrier);

	D3D12_CPU_DESCRIPTOR_HANDLE rtv = d3d12->getRenderTargetDescriptor();
	float clear[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	commandList->OMSetRenderTargets(1, &rtv, false, nullptr);
	commandList->ClearRenderTargetView(rtv, clear, 0, nullptr);
	commandList->SetGraphicsRootSignature(rootSignature.Get());
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

	D3D12_VIEWPORT viewport{ 0.0, 0.0, float(d3d12->getWindowWidth()), float(d3d12->getWindowHeight()), 0.0, 1.0 };
	D3D12_RECT scissor{ 0, 0, d3d12->getWindowWidth(), d3d12->getWindowHeight() };

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissor);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->DrawInstanced(3, 1, 0, 0);

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3d12->getCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	commandList->ResourceBarrier(1, &barrier);

	commandList->Close();

	ID3D12CommandList* commandLists[] = { commandList };
	d3d12->getCommandQueue()->ExecuteCommandLists(UINT(std::size(commandLists)), commandLists);

}

void Exercise2::createVertexBuffer()
{

	struct Vertex
	{
		float x, y, z;
	};

	Vertex vertices[3] =
	{
		{-1.0f, -1.0f, 0.0f },
		{ 0.0f,  1.0f, 0.0f },
		{ 1.0f, -1.0f, 0.0f }
	};

	vertexBuffer = app->getResources()->createDefaultBuffer(sizeof(vertices), vertices);

	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(vertices);
	vertexBufferView.StrideInBytes = sizeof(Vertex);

}

bool Exercise2::createRootSignature()
{

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> blob;
	if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, nullptr)))
		return false;

	if (FAILED(app->getD3D12()->getDevice()->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&rootSignature))))
		return false;

	return true;

}

void Exercise2::createPSO()
{

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = rootSignature.Get();

	auto dataVS = DX::ReadData(L"Exercise2VS.cso");
	auto dataPS = DX::ReadData(L"Exercise2PS.cso");

	psoDesc.VS = { dataVS.data(), dataVS.size() };
	psoDesc.PS = { dataPS.data(), dataPS.size() };

	D3D12_INPUT_ELEMENT_DESC inputLayout[] = { {"MY_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0} };
	psoDesc.InputLayout = { inputLayout, sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC) };

	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.NumRenderTargets = 1;
	psoDesc.SampleDesc = { 1, 0 };
	psoDesc.SampleMask = 0xffffffff;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	HRESULT hr = app->getD3D12()->getDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&PSO));
	if (FAILED(hr))
	{
		LOG("CreateGraphicsPipelineState failed: 0x%08X", hr);
	}
}