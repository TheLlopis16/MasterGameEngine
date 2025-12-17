#pragma once

#include "Module.h"
#include "DebugDrawPass.h"

class Exercise4 : public Module
{
	ComPtr<ID3D12Resource> textureDog;
	ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	ComPtr<ID3D12RootSignature> rootSignature;
	ComPtr<ID3D12PipelineState> PSO;
	std::unique_ptr<DebugDrawPass> debugDrawPass;
	UINT srvIndex = 0;

public:

	bool init() override;
	void render() override;

private:
	void createVertexBuffer();
	bool createRootSignature();
	void createPSO();
};