#pragma once
#include "Core/Mesh.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

struct MeshEntry
{
	int NumIndices;
	int BaseVertex;
	int BaseIndex;
};

class ModelLoader
{
	static Assimp::Importer importer;
	static ModelLoader*	Instance;
	ID3D12Device*		device;

	Mesh* Load(std::string filename, ID3D12GraphicsCommandList* clist);
	void ProcessNode(aiNode * node, const aiScene * scene,  ID3D12GraphicsCommandList* clist, Mesh* &outMesh);
	Mesh* ProcessMesh(UINT index, aiMesh* mesh, const aiScene * scene, Mesh* &outMesh, ID3D12GraphicsCommandList* clist);
	void ProcessMesh(UINT index, aiMesh* mesh, const aiScene * scene, std::vector<MeshEntry> meshEntries, std::vector<Vertex> &vertices, std::vector<UINT> &indices);
	void LoadBones(
		UINT index,
		const aiMesh* mesh, 
		const aiScene * scene,
		std::vector<MeshEntry> meshEntries,
		std::map<std::string, uint32_t> &boneMapping, 
		std::vector<BoneInfo> &boneInfoList, 
		std::vector<VertexBoneData> &bones
	);

	ModelLoader(ID3D12Device* device);
public:
	static Mesh* LoadFile(std::string filename, ID3D12GraphicsCommandList* clist);
	static ModelLoader* GetInstance();
	static ModelLoader* CreateInstance(ID3D12Device* device);
	static void DestroyInstance();
};

