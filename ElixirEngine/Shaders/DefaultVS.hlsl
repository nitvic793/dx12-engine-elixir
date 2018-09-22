struct VertexInput
{
	float3 pos		: POSITION;
	float2 uv		: TEXCOORD;
	float3 normal	: NORMAL;
	float3 tangent	: TANGENT;
};

struct VertexOutput
{
	float4 pos		: SV_POSITION;
	float2 uv		: TEXCOORD;
	float3 normal	: NORMAL;
	float3 tangent	: TANGENT;
};

cbuffer ConstantBuffer : register(b0)
{
	float4x4 worldViewProjection;
};

VertexOutput main(VertexInput input)
{
	VertexOutput output;
	output.pos = mul(float4(input.pos, 1.0f), worldViewProjection);
	output.uv = input.uv;
	output.normal = input.normal;
	output.tangent = input.tangent;
	return output;
}