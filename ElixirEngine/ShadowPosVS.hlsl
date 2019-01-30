cbuffer ConstantBuffer : register(b0)
{
	float4x4 world;
	float4x4 shadowView;
	float4x4 shadowProjection;
};

struct VertexShaderInput
{
	float3 position		: POSITION;
};

float4 main(VertexShaderInput input) : SV_POSITION
{
	float4x4 shadowVP = mul(mul(world, shadowView), shadowProjection);
	return  mul(float4(input.position, 1.0f), shadowVP);
}