#pragma once

#include "Module.h"
#include "Material.h"
#include "ImGuiPass.h"

class DebugDrawPass;
class Model;

class Exercise6 : public Module
{
	struct PerFrame
	{
		Vector3 lightDir = Vector3::UnitX;
		float pad0;
		Vector3 lightColor = Vector3::One;
		float pad1;
		Vector3 ambientColor = Vector3::Zero;
		float pad2;
		Vector3 viewPos = Vector3::Zero;
		float pad3;
	};

	struct PerInstance
	{
		Matrix modelMat;
		Matrix normalMat;

		PhongMaterialData material;
	};

	ComPtr<ID3D12RootSignature> rootSignature;
	ComPtr<ID3D12PipelineState> pso;
	DebugDrawPass* debugDraw = nullptr;
	ImGuiPass* imgui = nullptr;
	unsigned index = 0;
	int samplerIndex = 0;
	Model* model = nullptr;

public:
	Exercise6();
	~Exercise6();

	bool init() override;
	void render() override;
	void preRender() override;
	void update() override;

private:
	bool createRootSignature();
	bool createPSO();
	void imguiMaterial();

private:
	float kd = 1.0f;
	float ks = 0.3f;
	float shiny = 16.0f;
	float ambientValue = 0.1f;

	float lightDir[3] = { 0.7f, 0.7f, 0.7f };
	Vector3 ambient = Vector3(0.3f, 0.3f, 0.3f);

};