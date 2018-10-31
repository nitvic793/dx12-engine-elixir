TextureCube EnvMap			: register(t0);
SamplerState basicSampler	: register(s0);

cbuffer ExternalData : register(b0) {
	float roughness;
}

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 uvw			: TEXCOORD;
};

float4 main(VertexToPixel input) : SV_TARGET
{
	return float4(1, 0, 0, 1);
}