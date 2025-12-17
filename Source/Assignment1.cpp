#include "Globals.h"
#include "Assignment1.h"

#include "Application.h"
#include "ModuleD3D12.h"
#include "ModuleCamera.h"
#include "ModuleResources.h"
#include "ModuleShaderDescriptors.h"
#include "ReadData.h"
#include "ModuleSampler.h"

#include <d3d12.h>
#include "d3dx12.h"

bool Assignment1::init()
{
	createVertexBuffer();
	bool success = createRootSignature();
	createPSO();

	ModuleD3D12* d3d12 = app->getD3D12();
	ModuleResources* resources = app->getResources();
	ModuleShaderDescriptors* shaderDescriptors = app->getShaderDescriptors();
	debugDrawPass = std::make_unique<DebugDrawPass>(d3d12->getDevice(), d3d12->getCommandQueue(), false);
	imGuiPass = std::make_unique<ImGuiPass>(d3d12->getDevice(), d3d12->getHwnd());

	return true;
	textureDog = resources->createTextureFromFile(std::wstring(L"Assets/Textures/dog.dds"));
	shaderDescriptors->allocateDescriptor();
	srvIndex = shaderDescriptors->createSRV(textureDog.Get());


	return success;
}

void Assignment1::preRender()
{
	imGuiPass->startFrame();
}

void Assignment1::render()
{
	ImGui::Begin("Texture Viewer Options");
	ImGui::Checkbox("Show grid", &showGrid);
	ImGui::Checkbox("Show axis", &showAxis);
	ImGui::Combo("Sampler", &sampler, "Linear/Wrap\0Point/Wrap\0Linear/Clamp\0Point/Clamp", ModuleSampler::COUNT);
	ImGui::End();

	ModuleD3D12* d3d12 = app->getD3D12();
	ModuleCamera* camera = app->getCamera();
	ModuleShaderDescriptors* descriptors = app->getShaderDescriptors();
	ModuleSampler* samplers = app->getSamplers();
	ID3D12GraphicsCommandList* commandList = d3d12->getCommandList();
	ModuleShaderDescriptors* shaderDescriptors = app->getShaderDescriptors();

	commandList->Reset(d3d12->getCurrentCommandAllocator(), PSO.Get());
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3d12->getCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &barrier);

	unsigned width = d3d12->getWindowWidth();
	unsigned height = d3d12->getWindowHeight();
	Matrix model = Matrix::Identity;
	const Matrix view = camera->getView();
	Matrix proj = ModuleCamera::getPerspectiveProjection(float(width) / float(height));
	Matrix mvp = model * view * proj;
	mvp = mvp.Transpose();

	D3D12_VIEWPORT viewport;
	viewport.TopLeftX = viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.Width = float(width);
	viewport.Height = float(height);

	D3D12_RECT scissor;
	scissor.left = 0;
	scissor.top = 0;
	scissor.right = width;
	scissor.bottom = height;

	D3D12_CPU_DESCRIPTOR_HANDLE rtv = d3d12->getRenderTargetDescriptor();
	D3D12_CPU_DESCRIPTOR_HANDLE dsv = d3d12->getDepthStencilDescriptor();

	float clear[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	commandList->OMSetRenderTargets(1, &rtv, false, &dsv);
	commandList->ClearRenderTargetView(rtv, clear, 0, nullptr);
	commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);


	commandList->SetGraphicsRootSignature(rootSignature.Get());
	ID3D12DescriptorHeap* descriptorHeaps[] = { descriptors->getHeap(), samplers->getHeap() };
	commandList->SetGraphicsRoot32BitConstants(0, sizeof(Matrix) / sizeof(UINT32), &mvp, 0);

	commandList->SetDescriptorHeaps(2, descriptorHeaps);
	commandList->SetGraphicsRootDescriptorTable(1, shaderDescriptors->getGPUHandle(srvIndex));
	commandList->SetGraphicsRootDescriptorTable(2, samplers->getGPUHandle(ModuleSampler::Type(sampler)));

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissor);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->DrawInstanced(6, 1, 0, 0);

	if (showGrid) dd::xzSquareGrid(-10.0f, 10.0f, 0.0f, 1.0f, dd::colors::LightGray);
	if (showAxis) dd::axisTriad(ddConvert(Matrix::Identity), 0.1f, 2.0f);

	debugDrawPass->record(commandList, d3d12->getWindowWidth(), d3d12->getWindowHeight(), view, proj);
	imGuiPass->record(commandList);

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3d12->getCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	commandList->ResourceBarrier(1, &barrier);

	commandList->Close();

	ID3D12CommandList* commandLists[] = { commandList };
	d3d12->getCommandQueue()->ExecuteCommandLists(UINT(std::size(commandLists)), commandLists);

}

void Assignment1::createVertexBuffer()
{

	struct Vertex
	{
		Vector3 position;
		Vector2 uv;
	};

	static Vertex vertices[6] =
	{
		{ Vector3(-1.0f, -1.0f, 0.0f), Vector2(-0.2f, 1.2f) },
		{ Vector3(-1.0f, 1.0f, 0.0f), Vector2(-0.2f, -0.2f) },
		{ Vector3(1.0f, 1.0f, 0.0f), Vector2(1.2f, -0.2f) },
		{ Vector3(-1.0f, -1.0f, 0.0f), Vector2(-0.2f, 1.2f) },
		{ Vector3(1.0f, 1.0f, 0.0f), Vector2(1.2f, -0.2f) },
		{ Vector3(1.0f, -1.0f, 0.0f), Vector2(1.2f, 1.2f) }
	};

	vertexBuffer = app->getResources()->createDefaultBuffer(sizeof(vertices), vertices);

	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(vertices);
	vertexBufferView.StrideInBytes = sizeof(Vertex);

}

bool Assignment1::createRootSignature()
{

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	CD3DX12_ROOT_PARAMETER rootParameters[3] = {};
	CD3DX12_DESCRIPTOR_RANGE srvRange, sampRange;

	srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	sampRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, ModuleSampler::COUNT, 0);

	rootParameters[0].InitAsConstants((sizeof(Matrix) / sizeof(UINT32)), 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[2].InitAsDescriptorTable(1, &sampRange, D3D12_SHADER_VISIBILITY_PIXEL);

	rootSignatureDesc.Init(3, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> blob;
	if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, nullptr)))
		return false;

	if (FAILED(app->getD3D12()->getDevice()->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&rootSignature))))
		return false;

	return true;
}

void Assignment1::createPSO()
{

	D3D12_INPUT_ELEMENT_DESC inputLayout[] = { {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
											  {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0} };

	auto dataVS = DX::ReadData(L"Exercise4VS.cso");
	auto dataPS = DX::ReadData(L"Exercise4PS.cso");

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputLayout, sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC) };  // the structure describing our input layout
	psoDesc.pRootSignature = rootSignature.Get();                                                   // the root signature that describes the input data this pso needs
	psoDesc.VS = { dataVS.data(), dataVS.size() };                                                  // structure describing where to find the vertex shader bytecode and how large it is
	psoDesc.PS = { dataPS.data(), dataPS.size() };                                                  // same as VS but for pixel shader
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;                         // type of topology we are drawing
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;                                             // format of the render target
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc = { 1, 0 };                                                                    // must be the same sample description as the swapchain and depth/stencil buffer
	psoDesc.SampleMask = 0xffffffff;                                                                // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);                               // a default rasterizer state.
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);                                         // a default blend state.
	psoDesc.NumRenderTargets = 1;                                                                   // we are only binding one render target

	// create the pso
	app->getD3D12()->getDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&PSO));

}