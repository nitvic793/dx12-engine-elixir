#pragma once
#include "stdafx.h"

namespace Elixir
{
	typedef int NodeID;

	struct Node
	{
	public:

		//Actual Transforms
		XMFLOAT4X4			localTransform;
		XMFLOAT4X4			worldTransform;

		//Node information
		std::vector<NodeID>	children;
		NodeID				parent;

		Node();
		~Node();
	};

	void UpdateNode(Node& node, const Node& parentNode, const XMFLOAT3& position, const XMFLOAT3& scale, const XMFLOAT3& rotation);
	void UpdateRootNode(Node& node, const XMFLOAT3& position, const XMFLOAT3& scale, const XMFLOAT3& rotation);
	void UpdateNodes(Node* nodes, size_t count, XMFLOAT3* positions, XMFLOAT3* scales, XMFLOAT3* rotation);
}

