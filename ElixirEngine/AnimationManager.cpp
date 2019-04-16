#include "stdafx.h"
#include "AnimationManager.h"
#include "ResourceManager.h"
#include "Utility.h"

void AnimationManager::ReadNodeHeirarchy(uint32_t entityID, HashID meshID, UINT animationIndex, float AnimationTime)
{
	auto Animations = &animations[meshID];
	auto& boneData = boneDataMap[entityID];

	auto& boneDescriptor = boneData.MeshBoneDescriptor;
	auto& nodes = Animations->NodeHeirarchy;
	auto rootNode = Animations->RootNode;
	std::stack<std::string> nodeQueue;
	std::stack<XMFLOAT4X4> transformationQueue;
	auto globalInverse = XMLoadFloat4x4(&Animations->GlobalInverseTransform);
	auto rootTransform = XMMatrixIdentity();
	XMFLOAT4X4 identity;
	XMFLOAT4X4 globalFloat4x4;
	XMStoreFloat4x4(&identity, rootTransform);
	nodeQueue.push(rootNode);
	transformationQueue.push(identity);

	while (!nodeQueue.empty())
	{
		auto node = nodeQueue.top();
		auto parentTransformation = XMLoadFloat4x4(&transformationQueue.top());
		auto nodeTransformation = XMLoadFloat4x4(&Animations->NodeTransformsMap[node]);

		nodeQueue.pop();
		transformationQueue.pop();

		auto anim = Animations->GetChannel(animationIndex, node);
		if (anim != nullptr)
		{
			auto s = InterpolateScaling(AnimationTime, anim);
			auto scaling = XMMatrixScaling(s.x, s.y, s.z);

			auto r = InterpolateRotation(AnimationTime, anim);
			auto rotation = XMMatrixRotationQuaternion(XMVectorSet(r.y, r.z, r.w, r.x));

			auto t = InterpolatePosition(AnimationTime, anim);
			auto translation = XMMatrixTranslation(t.x, t.y, t.z);

			nodeTransformation += scaling * rotation * translation;
		}

		auto globalTransformation = nodeTransformation * parentTransformation;
		if (boneDescriptor.boneMapping.find(node) != boneDescriptor.boneMapping.end())
		{
			uint32_t BoneIndex = boneDescriptor.boneMapping[node];
			auto finalTransform = XMMatrixTranspose(OGLtoXM(boneDescriptor.boneInfoList[BoneIndex].Offset)) * globalTransformation * globalInverse;
			XMStoreFloat4x4(&boneDescriptor.boneInfoList[BoneIndex].FinalTransform, finalTransform);
		}

		auto children = Animations->NodeHeirarchy[node];
		for (int i = (int)children.size() - 1; i >= 0; --i)
		{
			XMStoreFloat4x4(&globalFloat4x4, globalTransformation);
			nodeQueue.push(children[i]);
			transformationQueue.push(globalFloat4x4);
		}
	}
}

AnimationManager::AnimationManager()
{
	animations = std::unordered_map<HashID, AnimationDescriptor>();
	resourceManager = ResourceManager::GetInstance();
}

void AnimationManager::RegisterMeshAnimations(HashID meshID, AnimationDescriptor* meshAnimations)
{
	animations.insert(std::pair<HashID, AnimationDescriptor>(meshID, *meshAnimations));
}

void AnimationManager::RegisterEntity(uint32_t entityID, HashID meshID)
{
	auto mesh = resourceManager->GetMesh(meshID);
	auto boneDescriptor = mesh->GetBoneDescriptor();
	auto boneData = BoneData 
	{	
		PerArmatureConstantBuffer{},
		boneDescriptor 
	};

	boneDataMap.insert(std::pair<uint32_t, BoneData>(entityID, boneData));
}

void AnimationManager::BoneTransform(uint32_t entityID, HashID meshID, UINT animationIndex, float totalTime, PerArmatureConstantBuffer* cb)
{
	auto animation = animations[meshID].GetAnimation(animationIndex);
	float TicksPerSecond = (float)(animation->TicksPerSecond != 0 ? animation->TicksPerSecond : 25.0f);
	float TimeInTicks = totalTime * TicksPerSecond;
	float AnimationTime = fmod(TimeInTicks, (float)animation->Duration);
	auto& boneDescriptor = boneDataMap[entityID].MeshBoneDescriptor;
	auto& boneCB = boneDataMap[entityID].ConstantBuffer;
	ReadNodeHeirarchy(entityID, meshID, animationIndex, AnimationTime);

	for (uint32_t i = 0; i < boneDescriptor.boneInfoList.size(); i++)
	{
		XMFLOAT4X4 finalTransform;
		XMStoreFloat4x4(&finalTransform, XMMatrixTranspose(XMLoadFloat4x4(&boneDescriptor.boneInfoList[i].FinalTransform)));
		cb->bones[i] = finalTransform;
	}
}

PerArmatureConstantBuffer* AnimationManager::GetConstantBuffer(uint32_t entityID)
{
	return &boneDataMap[entityID].ConstantBuffer;
}


AnimationManager::~AnimationManager()
{
}
