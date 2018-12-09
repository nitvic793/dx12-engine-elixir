
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
	float3 finalColor =  Sky.Sample(basicSampler, input.uvw).rgb;
	finalColor = finalColor / (finalColor + float3(1.f, 1.f, 1.f));
	float3 gammaCorrect = lerp(finalColor, pow(finalColor, 1.0 / 2.2), 0.4f);
	return float4(finalColor, 0.f);
}