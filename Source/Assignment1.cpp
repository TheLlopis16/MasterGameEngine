#include "Globals.h"
#include "Assignment1.h"
#include "Application.h"
#include "ModuleD3D12.h"
#include "ModuleResources.h"
#include "ReadData.h"
#include "DebugDrawPass.h"
#include "ModuleCamera.h"
#include "ModuleEditor.h"
#include "ModuleShaderDescriptors.h"
#include "ModuleSampler.h"

Assignment1::Assignment1() {}

Assignment1::~Assignment1()
{
	delete debugDraw;
}

bool Assignment1::init()
{

	struct Vertex
	{
		Vector3 position;
		Vector2 uv;
	};

	Vertex vertices[6] =
	{
		{ Vector3(-1.0f, -1.0f, 0.0f), Vector2(-0.2f, 1.2f) },
		{ Vector3(-1.0f, 1.0f, 0.0f), Vector2(-0.2f, -0.2f) },
		{ Vector3(1.0f, 1.0f, 0.0f), Vector2(1.2f, -0.2f) },
		{ Vector3(-1.0f, -1.0f, 0.0f), Vector2(-0.2f, 1.2f) },
		{ Vector3(1.0f, 1.0f, 0.0f), Vector2(1.2f, -0.2f) },
		{ Vector3(1.0f, -1.0f, 0.0f), Vector2(1.2f, 1.2f) }
	};

	texture = app->getResources()->createTextureFromFile(std::wstring(L"Assets/Textures/dog.dds"));
	index = app->getShaderDescriptors()->createSRV(texture.Get());

	vertexBuffer = app->getResources()->createDefaultBuffer(sizeof(vertices), vertices);

	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(vertices);
	vertexBufferView.StrideInBytes = sizeof(Vertex);

	debugDraw = new DebugDrawPass(app->getD3D12()->getDevice(), app->getD3D12()->getCommandQueue(), false);
	imgui = app->getEditor()->getImGui();

	bool succeed = createRootSignature();
	succeed = succeed && createPSO();

	return succeed;
}

void Assignment1::preRender()
{
	imgui->startFrame();
}

void Assignment1::render()
{

	ModuleD3D12* d3d12 = app->getD3D12();
	ModuleShaderDescriptors* descriptors = app->getShaderDescriptors();
	ID3D12GraphicsCommandList4* commandList = d3d12->getCommandList();
	ModuleCamera* camera = app->getCamera();
	ModuleSampler* samplers = app->getSamplers();

	ImGui::ShowDemoWindow();
	app->getEditor()->mainMenu();

	ImGui::Combo("Sampler", &samplerIndex, "Billinear Filtering Wrap\0Point Filtering Wrap\0Billinear Filtering Clamp\0Point Filtering Clamp", 4);

	commandList->Reset(d3d12->getCurrentCommandAllocator(), pso.Get());

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3d12->getCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &barrier);

	Matrix mvp = camera->getMVP();

	D3D12_CPU_DESCRIPTOR_HANDLE RTVhandle = d3d12->getRenderTargetDescriptor();
	D3D12_CPU_DESCRIPTOR_HANDLE DSVhandle = d3d12->getDepthStencilDescriptor();

	commandList->OMSetRenderTargets(1, &RTVhandle, false, &DSVhandle);
	float color[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	commandList->ClearDepthStencilView(DSVhandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	commandList->ClearRenderTargetView(RTVhandle, color, 0, nullptr);

	commandList->SetGraphicsRootSignature(rootSignature.Get());
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

	D3D12_VIEWPORT viewport{ 0.0, 0.0, float(d3d12->getWindowWidth()), float(d3d12->getWindowHeight()), 0.0, 1.0 };
	D3D12_RECT scissor{ 0, 0, d3d12->getWindowWidth(), d3d12->getWindowHeight() };

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissor);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRoot32BitConstants(0, sizeof(Matrix) / sizeof(UINT32), &mvp, 0);

	ID3D12DescriptorHeap* dh[] = { descriptors->getHeap(), samplers->getHeap() };
	commandList->SetDescriptorHeaps(2, dh);
	commandList->SetGraphicsRootDescriptorTable(1, descriptors->getGPUHandle(index));
	commandList->SetGraphicsRootDescriptorTable(2, samplers->getGPUHandle(samplerIndex));
	commandList->DrawInstanced(6, 1, 0, 0);


	dd::xzSquareGrid(-10.0f, 10.0f, 0.0f, 1.0f, dd::colors::LightGray);
	dd::axisTriad(ddConvert(Matrix::Identity), 0.1f, 1.0f);
	debugDraw->record(commandList, viewport.Width, viewport.Height, camera->getView(), camera->getProjection());

	imgui->record(commandList);

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3d12->getCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	commandList->ResourceBarrier(1, &barrier);
	commandList->Close();

	ID3D12CommandList* lists[] = { commandList };
	d3d12->getCommandQueue()->ExecuteCommandLists(1, lists);

}

bool Assignment1::createRootSignature()
{
	CD3DX12_ROOT_SIGNATURE_DESC signatureDesc = {};
	CD3DX12_ROOT_PARAMETER rootParameters[3] = {};
	CD3DX12_DESCRIPTOR_RANGE tableRange, sampRange;

	tableRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
	sampRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 0);

	rootParameters[0].InitAsConstants((sizeof(Matrix) / sizeof(UINT32)), 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsDescriptorTable(1, &tableRange, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[2].InitAsDescriptorTable(1, &sampRange, D3D12_SHADER_VISIBILITY_PIXEL);

	signatureDesc.Init(3, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> blob;

	bool succeed = SUCCEEDED(D3D12SerializeRootSignature(&signatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, nullptr));
	succeed = succeed && SUCCEEDED(app->getD3D12()->getDevice()->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));

	return succeed;
}

bool Assignment1::createPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
	desc.pRootSignature = rootSignature.Get();

	auto dataVS = DX::ReadData(L"Assignment1VS.cso");
	auto dataPS = DX::ReadData(L"Assignment1PS.cso");

	desc.VS = { dataVS.data(), dataVS.size() };
	desc.PS = { dataPS.data(), dataPS.size() };

	D3D12_INPUT_ELEMENT_DESC inputLayout[] = { {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0} };
	desc.InputLayout = { inputLayout, sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC) };

	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.NumRenderTargets = 1;
	desc.SampleDesc = { 1, 0 };
	desc.SampleMask = 0xffffffff;
	desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	bool succeed = SUCCEEDED(app->getD3D12()->getDevice()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pso)));

	return succeed;
}