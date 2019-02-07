#pragma once
#include "Core/Mesh.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

class ModelLoader
{
	static ModelLoader*	Instance;
	ID3D12Device*		device;

	std::vector<Mesh*> Load(std::string filename, ID3D12GraphicsCommandList* clist);
	void ProcessNode(aiNode * node, const aiScene * scene,  ID3D12GraphicsCommandList* clist, std::vector<Mesh*> &meshes);
	Mesh* ProcessMesh(aiMesh* mesh, const aiScene * scene,  ID3D12GraphicsCommandList* clist);
	ModelLoader(ID3D12Device* device);
public:
	static std::vector<Mesh*> LoadFile(std::string filename, ID3D12GraphicsCommandList* clist);
	static ModelLoader* GetInstance();
	static ModelLoader* CreateInstance(ID3D12Device* device);
};

