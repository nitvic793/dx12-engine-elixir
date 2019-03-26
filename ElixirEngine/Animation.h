#pragma once

#include "stdafx.h"
#include <unordered_map>

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
	std::unordered_map<std::string, std::vector<std::string>> NodeHeirarchy;
	std::vector<Animation> Animations;
};