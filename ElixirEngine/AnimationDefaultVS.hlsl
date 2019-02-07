#define MAX_BONES 128

struct VertexAnimatedInput
{
	float3 pos			: POSITION;
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	uint4  skinIndices	: BLENDINDICES;
	float4 skinWeights	: BLENDWEIDHT;
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

cbuffer ConstantBuffer : register(b0)
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
	float4x4 bones[MAX_BONES];
}

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

VertexOutput main(VertexAnimatedInput input)
{
	VertexOutput output;
	float4x4 shadowVP = mul(mul(world, shadowView), shadowProjection);

	output.pos = mul(float4(input.pos, 1.0f), worldViewProjection);
	output.uv = input.uv;
	output.normal = normalize(mul(input.normal, (float3x3)world));
	output.tangent = normalize(mul(input.tangent, (float3x3)world));
	output.worldPos = mul(float4(input.pos, 1.0f), world).xyz;
	output.linearZ = LinearZ(output.pos);
	output.shadowPos = mul(float4(input.pos, 1.0f), shadowVP);
	return output;
}