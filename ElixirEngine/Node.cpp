#include "stdafx.h"
#include "Node.h"
#include <queue>
using namespace Elixir;

Node::Node()
{
}


Node::~Node()
{
}

XMMATRIX XM_CALLCONV GetTransformation(const XMFLOAT3 & position, const XMFLOAT3 & scale, const XMFLOAT3 & rotation)
{
	XMMATRIX translationM = XMMatrixTranslationFromVector(XMLoadFloat3(&position));
	XMMATRIX rotationM = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotation));
	XMMATRIX scaleM = XMMatrixScalingFromVector(XMLoadFloat3(&scale));
	auto transformation =  scaleM * rotationM * translationM;
	return transformation;
}

void Elixir::UpdateNode(Node & node, const Node& parentNode, const XMFLOAT3 & position, const XMFLOAT3 & scale, const XMFLOAT3 & rotation)
{
	auto transformation = GetTransformation(position, scale, rotation);
	XMStoreFloat4x4(&node.localTransform, transformation);
	transformation = transformation * XMLoadFloat4x4(&parentNode.worldTransform) ;
	XMStoreFloat4x4(&node.worldTransform, transformation);
}

void Elixir::UpdateRootNode(Node & node, const XMFLOAT3 & position, const XMFLOAT3 & scale, const XMFLOAT3 & rotation)
{
	auto transformation = GetTransformation(position, scale, rotation);
	XMStoreFloat4x4(&node.localTransform, transformation);
	node.worldTransform = node.localTransform;
}

void Elixir::UpdateNodes(Node* nodes, size_t count, XMFLOAT3 * positions, XMFLOAT3 * scales, XMFLOAT3 * rotations)
{
	UpdateRootNode(nodes[0], positions[0], scales[0], rotations[0]);
	std::queue<NodeID> nodeQueue;
	for (auto child : nodes[0].children) //Skip root node
	{
		nodeQueue.push(child);
	}

	while (!nodeQueue.empty()) //TODO: Serialize this further to update nodes without queue
	{
		auto nodeId = nodeQueue.front();
		nodeQueue.pop();
		auto &node = nodes[nodeId];
		auto parent = nodes[node.parent];
		UpdateNode(node, parent, positions[nodeId], scales[nodeId], rotations[nodeId]);
		for (auto child : node.children) //Skip root node
		{
			nodeQueue.push(child);
		}
	}
}
