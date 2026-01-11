#include "Globals.h"
#include "Exercise6.h"
#include "Application.h"
#include "ModuleD3D12.h"
#include "ModuleResources.h"
#include "ReadData.h"
#include "DebugDrawPass.h"
#include "ModuleCamera.h"
#include "ModuleEditor.h"
#include "ModuleShaderDescriptors.h"
#include "ModuleSampler.h"
#include "Model.h"
#include "ImGuizmo.h"
#include "ModuleRingBuffer.h"

Exercise6::Exercise6() {}

Exercise6::~Exercise6()
{
	delete debugDraw, model;
}

bool Exercise6::init()
{

	model = new Model();
	model->Load("Assets/Models/Duck/Duck.gltf", "Assets/Models/Duck/");
	//model->Load("Assets/Models/Box/BoxTextured.gltf", "Assets/Models/Box/");

	debugDraw = new DebugDrawPass(app->getD3D12()->getDevice(), app->getD3D12()->getCommandQueue(), false);
	imgui = app->getEditor()->getImGui();

	bool succeed = createRootSignature();
	succeed = succeed && createPSO();

	return succeed;
}

void Exercise6::preRender()
{
	ModuleRingBuffer* ringBuffer = app->getRingBuffer();

	ringBuffer->preRender();

	imgui->startFrame();
	ImGuizmo::BeginFrame();
}

void Exercise6::update()
{
	ModuleCamera* camera = app->getCamera();

	Matrix modelMatrix = model->getModelMatrix();
	ModuleEditor* editor = app->getEditor();

	if (editor->getSamplerIndex() != samplerIndex)
		samplerIndex = editor->getSamplerIndex();

	camera->setModelMatrix(modelMatrix);
}

void Exercise6::render()
{
	ModuleD3D12* d3d12 = app->getD3D12();
	ModuleShaderDescriptors* descriptors = app->getShaderDescriptors();
	ID3D12GraphicsCommandList4* commandList = d3d12->getCommandList();
	ModuleCamera* camera = app->getCamera();
	ModuleSampler* samplers = app->getSamplers();
	ModuleRingBuffer* ringBuffer = app->getRingBuffer();


	app->getEditor()->mainMenu();

	app->getEditor()->modelOptions(*model);

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


	D3D12_VIEWPORT viewport{ 0.0, 0.0, float(d3d12->getWindowWidth()), float(d3d12->getWindowHeight()), 0.0, 1.0 };
	D3D12_RECT scissor{ 0, 0, d3d12->getWindowWidth(), d3d12->getWindowHeight() };
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissor);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->SetGraphicsRootSignature(rootSignature.Get());

	ID3D12DescriptorHeap* dh[] = { descriptors->getHeap(), samplers->getHeap() };
	commandList->SetDescriptorHeaps(2, dh);

	PerFrame pf;
	pf.lightDir = Vector3(lightDir[0], lightDir[1], lightDir[2]);
	pf.ambientColor = ambient * ambientValue;
	pf.viewPos = camera->getPosition();

	pf.lightDir.Normalize();

	RingAlloc pfAlloc = ringBuffer->allocBuffer(sizeof(PerFrame));
	memcpy(pfAlloc.cpu, &pf, sizeof(PerFrame));

	commandList->SetGraphicsRootConstantBufferView(1, pfAlloc.gpu);


	commandList->SetGraphicsRoot32BitConstants(0, sizeof(Matrix) / sizeof(UINT32), &mvp, 0);

	imguiMaterial();


	for (uint32_t i = 0; i < model->getNumMeshes(); ++i)
	{
		Mesh& mesh = model->getMesh(i);

		commandList->IASetVertexBuffers(0, 1, mesh.getVBV());

		int matIdx = mesh.getMatIndex();
		if (matIdx < 0) matIdx = 0;
		Material& mat = model->getMaterial(matIdx);
		
		Matrix modelMat = model->getModelMatrix();
		Matrix normalMat = modelMat.Invert().Transpose();

		PerInstance pi = {};
		pi.modelMat = modelMat;
		pi.normalMat = normalMat;
		mat.setPhong(kd, ks, shiny);
		pi.material = mat.getPhong();

		RingAlloc piAlloc = ringBuffer->allocBuffer(sizeof(PerInstance));
		memcpy(piAlloc.cpu, &pi, sizeof(PerInstance));

		commandList->SetGraphicsRootConstantBufferView(2, piAlloc.gpu);

		commandList->SetGraphicsRootDescriptorTable(3, descriptors->getGPUHandle(mat.getIndex()));

		commandList->SetGraphicsRootDescriptorTable(4, samplers->getGPUHandle(samplerIndex));

		if (mesh.isIndexed())
		{
			const auto& ibv = mesh.getIBV();
			commandList->IASetIndexBuffer(ibv);
			commandList->DrawIndexedInstanced(mesh.getIndicesNum(), 1, 0, 0, 0);
		}
		else
		{
			commandList->DrawInstanced(mesh.getVerticesNum(), 1, 0, 0);
		}
	}

	dd::xzSquareGrid(-10.0f, 10.0f, 0.0f, 1.0f, dd::colors::LightGray);
	debugDraw->record(commandList, viewport.Width, viewport.Height, camera->getView(), camera->getProjection());

	imgui->record(commandList);

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3d12->getCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	commandList->ResourceBarrier(1, &barrier);
	commandList->Close();

	ID3D12CommandList* lists[] = { commandList };
	d3d12->getCommandQueue()->ExecuteCommandLists(1, lists);
}

bool Exercise6::createRootSignature()
{
	CD3DX12_ROOT_SIGNATURE_DESC signatureDesc = {};
	CD3DX12_ROOT_PARAMETER rootParameters[5] = {};
	CD3DX12_DESCRIPTOR_RANGE tableRange, sampRange;

	tableRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
	sampRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 4, 0, 0);

	rootParameters[0].InitAsConstants((sizeof(Matrix) / sizeof(UINT32)), 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[2].InitAsConstantBufferView(2, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[3].InitAsDescriptorTable(1, &tableRange, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[4].InitAsDescriptorTable(1, &sampRange, D3D12_SHADER_VISIBILITY_PIXEL);

	signatureDesc.Init(5, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> blob;

	bool succeed = SUCCEEDED(D3D12SerializeRootSignature(&signatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, nullptr));
	succeed = succeed && SUCCEEDED(app->getD3D12()->getDevice()->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));

	return succeed;
}

bool Exercise6::createPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
	desc.pRootSignature = rootSignature.Get();

	auto dataVS = DX::ReadData(L"Exercise6VS.cso");
	auto dataPS = DX::ReadData(L"Exercise6PS.cso");

	desc.VS = { dataVS.data(), dataVS.size() };
	desc.PS = { dataPS.data(), dataPS.size() };

	D3D12_INPUT_ELEMENT_DESC inputLayout[] = { {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0} };
	desc.InputLayout = { inputLayout, sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC) };

	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.NumRenderTargets = 1;
	desc.SampleDesc = { 1, 0 };
	desc.SampleMask = 0xffffffff;
	desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	desc.RasterizerState.FrontCounterClockwise = TRUE;
	desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	bool succeed = SUCCEEDED(app->getD3D12()->getDevice()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pso)));

	return succeed;
}

void Exercise6::imguiMaterial()
{
	ImGui::Begin("Material Options");
	ImGui::DragFloat("Diffuse: ", &kd, 0.01f, 0.1f, 1.0f, "%.2f");
	ImGui::DragFloat("Specular: ", &ks, 0.01f, 0.0f, 10.0f, "%.2f");
	ImGui::DragFloat("Shininess: ", &shiny, 1.0f, 0.1f, 128.0f, "%1.f");
	ImGui::Separator();
	ImGui::Text("Light Options");
	ImGui::DragFloat("Light X: ", &lightDir[0], 0.1f, -10.0f, 10.0f, "%.1f");
	ImGui::DragFloat("Light Y: ", &lightDir[1], 0.1f, -10.0f, 10.0f, "%.1f");
	ImGui::DragFloat("Light Z: ", &lightDir[2], 0.1f, -10.0f, 10.0f, "%.1f");
	ImGui::DragFloat("Ambient : ", &ambientValue, 0.01f, 0.0f, 0.3f, "%.2f");	
	ImGui::End();

}