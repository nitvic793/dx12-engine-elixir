struct VertexInput
{
	float3 pos : POSITION;
	float4 color: COLOR;
};

struct VertexOutput
{
	float4 pos: SV_POSITION;
	float4 color: COLOR;
};

cbuffer ConstantBuffer : register(b0)
{
	float4 colorMultiplier;
};

VertexOutput main(VertexInput input)
{
	VertexOutput output;
	output.pos = float4(input.pos, 1.0f);
	output.color = input.color * colorMultiplier;
	return output;
}