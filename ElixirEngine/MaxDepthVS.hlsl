cbuffer externalData : register(b0)
{
	matrix wvp; //Not used
	matrix world;
	matrix view;
	matrix projection;
	matrix sview; //not used
	matrix sprojection; //not used
};

// Struct representing a single vertex worth of data
struct VertexShaderInput
{
	float3 position		: POSITION;
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
};


float4 main(VertexShaderInput input) : SV_POSITION
{
	// Calculate output position
	matrix worldViewProj = mul(mul(world, view), projection);
	//input.position = input.position + normalize(input.normal) * 0.01f;
	float4 position = mul(float4(input.position, 1.0f), worldViewProj);
	position.z = position.w;
	return position;
}