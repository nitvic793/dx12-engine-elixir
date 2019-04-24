#pragma once
#include "SceneCommon.h"
#include "Node.h"

namespace Elixir
{
	class Scene
	{
	protected:
		NodeID					rootNode; //Would be 0 always
		std::vector<Node>		nodeList;

		//Transform Data
		std::vector<XMFLOAT3>	position;
		std::vector<XMFLOAT3>	rotation;
		std::vector<XMFLOAT3>	scale;
		std::vector<byte>		isActive;

		const Node				CreateNode(Transform transform);
		void					InsertTransform(Transform transform);
	public:
		Scene();
		NodeID					CreateNode(NodeID parent, Transform transform = DefaultTransform);
		inline void				UpdateTransforms() { UpdateNodes(nodeList.data(), nodeList.size(), position.data(), scale.data(), rotation.data(), isActive.data()); };

		void					SetTransform(NodeID nodeId, const Transform& transform);
		void					SetTranslation(NodeID nodeId, const XMFLOAT3& translation);
		void					SetRotation(NodeID nodeId, const XMFLOAT3& rotationV);
		void					SetScale(NodeID nodeId, const XMFLOAT3& scaleV);
		void					SetActive(NodeID nodeId, bool enabled);

		const bool				IsActive(NodeID nodeId);
		const XMFLOAT3&			GetTranslation(NodeID nodeId);
		const XMFLOAT3&			GetRotation(NodeID nodeId);
		const XMFLOAT3&			GetScale(NodeID nodeId);
		const XMFLOAT4X4&		GetTransformMatrix(NodeID nodeId);
		Transform				GetTransform(NodeID nodeId);
		~Scene();
	};
}

