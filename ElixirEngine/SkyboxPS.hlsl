
TextureCube Sky				: register(t0);
SamplerState basicSampler	: register(s0);

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 uvw			: TEXCOORD;
};


// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	return float4(Sky.Sample(basicSampler, input.uvw).rgb, 0.f);
}