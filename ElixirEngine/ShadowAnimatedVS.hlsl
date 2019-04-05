cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
};

// Struct representing a single vertex worth of data
struct VertexShaderInput
{
	float3 position		: POSITION;
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	uint4  skinIndices	: BLENDINDICES;
	float4 skinWeights	: BLENDWEIGHT;
};

cbuffer PerArmature: register(b1)
{
	matrix bones[128];
};

float4x4 SkinTransform(float4 weights, uint4 boneIndices)
{
	// Calculate the skin transform from up to four bones and weights
	float4x4 skinTransform =
		bones[boneIndices.x] * weights.x +
		bones[boneIndices.y] * weights.y +
		bones[boneIndices.z] * weights.z +
		bones[boneIndices.w] * weights.w;
	return skinTransform;
}

void SkinVertex(inout float4 position, inout float3 normal, float4x4 skinTransform)
{
	position = mul(position, skinTransform);
	normal = mul(normal, (float3x3)skinTransform);
}


float4 main(VertexShaderInput input) : SV_POSITION
{
	// Calculate output position
	float4x4 skinTransform = SkinTransform(input.skinWeights, input.skinIndices);
	float4 position = float4(input.position, 1.0f);
	if (input.skinWeights.x != 0)
	{
		SkinVertex(position, input.normal, skinTransform);
	}

	matrix worldViewProj = mul(mul(world, view), projection);
	return mul(position, worldViewProj);
}