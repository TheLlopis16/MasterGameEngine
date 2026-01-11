#include "Globals.h"
#include "Model.h"
#include "Mesh.h"
#include "Material.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"

Model::Model() {}

Model::~Model() {}

void Model::Load(const char* assetFileName, const char* assetFolder)
{
	tinygltf::TinyGLTF gltfObject;
	tinygltf::Model model;
	std::string error, warning;

	bool loaded = gltfObject.LoadASCIIFromFile(&model, &error, &warning, assetFileName);

	if (loaded)
	{

		materialNum = model.materials.size();
		materials = std::make_unique<Material[]>(materialNum);
		int matIndex = 0;

		for (const tinygltf::Material& material : model.materials)
		{
			materials[matIndex++].load(model, material, assetFolder);
		}

		meshesNum = 0;

		for (const tinygltf::Mesh& mesh : model.meshes)
		{
			meshesNum += mesh.primitives.size();
		}

		meshes = std::make_unique<Mesh[]>(meshesNum);

		int meshIndex = 0;

		for (const tinygltf::Mesh& mesh : model.meshes)
		{
			for (const tinygltf::Primitive& primitive : mesh.primitives)
			{
				meshes[meshIndex++].load(model, mesh, primitive);
			}
		}
	}
}