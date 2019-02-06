cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
};

// Struct representing a single vertex worth of data
struct VertexShaderInput
{
	float3		position		: POSITION;
	float2		uv				: TEXCOORD;
	float3		normal			: NORMAL;
	float3		tangent			: TANGENT;
	float4x4	instanceWorld	: WORLD_INSTANCE;
};


float4 main(VertexShaderInput input) : SV_POSITION
{
	// Calculate output position
	matrix worldViewProj = mul(mul(input.instanceWorld, view), projection);
	return mul(float4(input.position, 1.0f), worldViewProj);
}