#pragma once

#include "Module.h"
#include "DebugDrawPass.h"
#include "ModuleSampler.h"
#include "ImGuiPass.h"

class ModuleEditor;

class Assignment1 : public Module
{
	ComPtr<ID3D12Resource> textureDog;
	ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	ComPtr<ID3D12RootSignature> rootSignature;
	ComPtr<ID3D12PipelineState> PSO;
	std::unique_ptr<DebugDrawPass> debugDrawPass;
	ImGuiPass* imGuiPass = nullptr;
	UINT srvIndex = 0;
	int sampler = int(ModuleSampler::LINEAR_WRAP);
	bool showGrid = true;
	bool showAxis = true;
	bool freeLook = false;

public:

	bool init() override;
	void preRender() override;
	void render() override;

private:
	void createVertexBuffer();
	bool createRootSignature();
	void createPSO();
};