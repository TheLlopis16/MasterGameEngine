#pragma once
#include "Module.h"

class Exercise2 : public Module
{
	ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	ComPtr<ID3D12RootSignature> rootSignature;
	ComPtr<ID3D12PipelineState> PSO;

public:
	bool init() override;
	void render() override;

private:
	void createVertexBuffer();
	bool createRootSignature();
	void createPSO();
};