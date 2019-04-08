#pragma once
#include "stdafx.h"

namespace Elixir
{
	XMFLOAT4X4 GetWorldViewProjectionTransposed(const XMFLOAT4X4& world, const XMFLOAT4X4& view, const XMFLOAT4X4& projection);
	XMFLOAT4X4 Transpose(const XMFLOAT4X4& matrix);
}