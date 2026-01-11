#pragma once

#include "Module.h"
#include "ImGuiPass.h"

class DebugDrawPass;
class Model;

class Exercise5 : public Module
{
	ComPtr<ID3D12RootSignature> rootSignature;
	ComPtr<ID3D12PipelineState> pso;
	DebugDrawPass* debugDraw = nullptr;
	ImGuiPass* imgui = nullptr;
	unsigned index = 0;
	int samplerIndex = 0;
	Model* model = nullptr;

	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

public:
	Exercise5();
	~Exercise5();

	bool init() override;
	void render() override;
	void preRender() override;
	void update() override;

private:
	bool createRootSignature();
	bool createPSO();

};