/*#pragma once

#include "Module.h"
#include "DebugDrawPass.h"

class Exercise3 : public Module
{
	ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	ComPtr<ID3D12RootSignature> rootSignature;
	ComPtr<ID3D12PipelineState> PSO;
	std::unique_ptr<DebugDrawPass> debugDrawPass;

public:

	bool init() override;
	void render() override;

private:
	void createVertexBuffer();
	bool createRootSignature();
	void createPSO();
};*/