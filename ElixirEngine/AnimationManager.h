#pragma once

#include "stdafx.h"
#include "Animation.h"
#include "Core/ConstantBuffer.h"
#include "../Engine.Serialization/StringHash.h"
#include "Core/Mesh.h"
#include <unordered_map>

class ResourceManager;

struct BoneData
{
	PerArmatureConstantBuffer	ConstantBuffer;
	BoneDescriptor				MeshBoneDescriptor;
};

class AnimationManager
{
	std::unordered_map<HashID, AnimationDescriptor> animations;
	std::unordered_map<uint32_t, BoneData> boneDataMap;
	ResourceManager* resourceManager;

	void ReadNodeHeirarchy(uint32_t entityID, HashID meshID, UINT animationIndex, float AnimationTime);
public:
	AnimationManager();
	void RegisterMeshAnimations(HashID meshID, AnimationDescriptor* meshAnimations);
	void RegisterEntity(uint32_t entityID, HashID meshID);
	void BoneTransform(uint32_t entityID, HashID meshID, UINT animationIndex, float totalTime, PerArmatureConstantBuffer* cb);
	PerArmatureConstantBuffer* GetConstantBuffer(uint32_t entityID);
	~AnimationManager();
};


//Immediate TODO:
// Finish above class
// Create POD for entities

