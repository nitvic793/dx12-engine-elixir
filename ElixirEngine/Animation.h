#pragma once

#include "stdafx.h"
#include <unordered_map>
#include "OGLMath.h"

struct VectorKey
{
	double Time;
	XMFLOAT3 Value;
};

struct QuaternionKey
{
	double Time;
	XMFLOAT4 Value;
};

struct AnimationChannel
{
	std::string NodeName;
	std::vector<VectorKey> PositionKeys;
	std::vector<QuaternionKey> RotationKeys;
	std::vector<VectorKey> ScalingKeys;
};

struct Animation
{
	std::vector<AnimationChannel> Channels;
	std::unordered_map<std::string, uint32_t> NodeChannelMap;
};

struct AnimationDescriptor
{
	std::string RootNode;
	XMFLOAT4X4 GlobalInverseTransform;
	std::unordered_map<std::string, std::vector<std::string>> NodeHeirarchy;
	std::unordered_map<std::string, XMFLOAT4X4> NodeTransformsMap;
	std::vector<Animation> Animations;
};