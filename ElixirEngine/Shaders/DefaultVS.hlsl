struct VertexInput
{
	float4 pos : POSITION;
	float2 uv  : TEXCOORD;
};

struct VertexOutput
{
	float4 pos: SV_POSITION;
	float2 uv: TEXCOORD;
};

cbuffer ConstantBuffer : register(b0)
{
	float4x4 worldViewProjection;
};

VertexOutput main(VertexInput input)
{
	VertexOutput output;
	output.pos = mul(input.pos, worldViewProjection);
	output.uv = input.uv;
	return output;
}