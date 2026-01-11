#pragma once

namespace tinygltf { class Model; struct Material; }

struct PhongMaterialData
{
	Vector4 diffuseColor;
	float Kd;
	float Ks;
	float shininess;
	BOOL hasDiffuseTex;
};


class Material
{
	struct MaterialData
	{
		Vector4 baseColor;
		BOOL hasColorTexture;
	};

	//MaterialData data;
	PhongMaterialData phong;
	ComPtr<ID3D12Resource> colorTexture;
	unsigned index = 0;

	//ComPtr<ID3D12Resource> materialBuffer;
	//D3D12_GPU_VIRTUAL_ADDRESS materialAddress;

public:
	Material();
	~Material();

	void load(const tinygltf::Model& model, const tinygltf::Material& material, const char* basePath);

	//const D3D12_GPU_VIRTUAL_ADDRESS getMatGPUAddress() { return materialAddress; }
	const PhongMaterialData& getPhong() const { return this->phong; }
	void setPhong(float kd, float ks, float shiny);

	ID3D12Resource* getColorTexture() { return colorTexture.Get(); }

	const unsigned& getIndex() const { return this->index; }
};