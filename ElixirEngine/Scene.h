#pragma once
#include "Node.h"

namespace Elixir
{
	struct Transform
	{
		XMFLOAT3 Position;
		XMFLOAT3 Rotation;
		XMFLOAT3 Scale;
		
		static Transform Create(XMFLOAT3 position = XMFLOAT3(0, 0, 0), XMFLOAT3 rotation = XMFLOAT3(0, 0, 0), XMFLOAT3 scale = XMFLOAT3(1.f, 1.f, 1.f))
		{
			Transform transform;
			transform.Position = position;
			transform.Rotation = rotation;
			transform.Scale = scale;
			return transform;
		}
	};

	static const Transform DefaultTransform =
	{
		{}, {}, {1.f,1.f,1.f}
	};

	class Scene
	{
	protected:
		NodeID					rootNode; //Would be 0 always
		std::vector<Node>		nodeList;

		//Transform Data
		std::vector<XMFLOAT3>	position;
		std::vector<XMFLOAT3>	rotation;
		std::vector<XMFLOAT3>	scale;
		std::vector<bool>		isDirty;

		const Node				CreateNode(Transform transform);
		void					InsertTransform(Transform transform);
	public:
		Scene();
		NodeID					CreateNode(NodeID parent, Transform transform = DefaultTransform);
		inline void				UpdateTransforms() { UpdateNodes(nodeList.data(), nodeList.size(), position.data(), scale.data(), rotation.data()); };

		void					SetTransform(NodeID nodeId, const Transform& transform);
		~Scene();
	};
}

