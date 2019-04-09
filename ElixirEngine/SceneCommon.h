#pragma once

namespace Elixir
{
	typedef int32_t EntityID;
	typedef int32_t NodeID;
	typedef size_t	TypeID;
	struct Transform
	{
		XMFLOAT3 Position;
		XMFLOAT3 Rotation;
		XMFLOAT3 Scale;

		static Transform Create(const XMFLOAT3& position = XMFLOAT3(0, 0, 0), const XMFLOAT3& rotation = XMFLOAT3(0, 0, 0), const XMFLOAT3& scale = XMFLOAT3(1.f, 1.f, 1.f))
		{
			Transform transform;
			transform.Position = position;
			transform.Rotation = rotation;
			transform.Scale = scale;
			return transform;
		}
	};

	struct TransformSOA
	{
		XMFLOAT3*	Position;
		XMFLOAT3*	Rotation;
		XMFLOAT3*	Scale;
		size_t		Count;
	};

	static const Transform DefaultTransform =
	{
		{}, {}, {1.f,1.f,1.f}
	};

}
