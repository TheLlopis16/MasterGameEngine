#pragma once

#include "Mesh.h"
#include "Material.h"

class Model
{
	std::unique_ptr<Material[]> materials;
	std::unique_ptr<Mesh[]> meshes;

	size_t materialNum = 0;
	size_t meshesNum = 0;

	Matrix model = Matrix::Identity;
	
public:
	Model();
	~Model();

	void Load(const char* assetFileName, const char* assetFolder);

	Material& getMaterial(size_t idx) { return materials[idx]; }
	size_t getNumMaterials() const { return materialNum; }

	Mesh& getMesh(size_t idx) { return meshes[idx]; }
	size_t getNumMeshes() const { return meshesNum; }

	void setModelMatrix(const Matrix& m) { model = m; }
	Matrix& getModelMatrix() { return model; }

};