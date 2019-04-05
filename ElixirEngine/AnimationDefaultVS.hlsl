#define MAX_BONES 128

struct VertexAnimatedInput
{
	float3 pos			: POSITION;
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	uint4  skinIndices	: BLENDINDICES;
	float4 skinWeights	: BLENDWEIGHT;
};

struct VertexOutput
{
	float4 pos			: SV_POSITION;
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float3 worldPos		: POSITION;
	float  linearZ		: LINEARZ;
	float4 shadowPos	: SHADOWPOS;
};

cbuffer PerEntity : register(b0)
{
	float4x4 worldViewProjection;
	float4x4 world;
	float4x4 view;
	float4x4 projection;
	float4x4 shadowView;
	float4x4 shadowProjection;
};

cbuffer PerFrame : register(b1)
{
	float nearZ;
	float farZ;
	float2 lightPerspectiveValues;
};

cbuffer PerArmature: register(b2)
{
	matrix bones[MAX_BONES];
};

float2 ProjectionConstants(float nearZ, float farZ)
{
	float2 projectionConstants;
	projectionConstants.x = farZ / (farZ - nearZ);
	projectionConstants.y = (-farZ * nearZ) / (farZ - nearZ);
	return projectionConstants;
}

float LinearZ(float4 outPosition)
{
	float2 projectionConstants = ProjectionConstants(nearZ, farZ);
	float depth = outPosition.z / outPosition.w;
	float linearZ = projectionConstants.y / (depth - projectionConstants.x);
	return linearZ;
}

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

VertexOutput main(VertexAnimatedInput input)
{
	VertexOutput output;

	float4x4 skinTransform = SkinTransform(input.skinWeights, input.skinIndices);
	float4 position = float4(input.pos, 1.0f);
	if (input.skinWeights.x != 0)
	{
		SkinVertex(position, input.normal, skinTransform);
	}

	float4x4 shadowVP = mul(mul(world, shadowView), shadowProjection);
	output.pos = mul(position, worldViewProjection);
	output.uv = input.uv;
	output.normal = normalize(mul(input.normal, (float3x3)world));
	output.tangent = normalize(mul(input.tangent, (float3x3)world));
	output.worldPos = mul(float4(input.pos, 1.f), world).xyz;
	output.linearZ = LinearZ(output.pos);
	output.shadowPos = mul(float4(input.pos, 1.f), shadowVP);
	return output;
}