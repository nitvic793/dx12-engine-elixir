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
	float3 worldPos	: POSITION;
};

cbuffer ConstantBuffer : register(b0)
{
	float4x4 worldViewProjection;
	float4x4 world;
	float4x4 view;
	float4x4 projection;
};

VertexOutput main(VertexInput input)
{
	VertexOutput output;
	output.pos = mul(float4(input.pos, 1.0f), worldViewProjection);
	output.uv = input.uv;
	//output.normal = input.normal;
	output.normal = normalize(mul(input.normal, (float3x3)world));
	output.tangent = input.tangent;
	output.worldPos = mul(float4(input.pos, 1.0f), world).xyz;
	return output;
}