cbuffer externalData : register(b0)
{
	matrix world;
	matrix view; // not used
	matrix projection; // not used
};
//TODO: Optimize Constant Buffer

struct VertexInput
{
	float4 pos : POSITION;

	float4x4 instanceWorld: WORLD_INSTANCE;
};

float4 main(VertexInput input) : SV_POSITION
{
	return mul(float4(input.pos.xyz, 1.0f), input.instanceWorld);
}