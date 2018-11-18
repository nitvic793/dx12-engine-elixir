struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

Texture2D OcclusionTex		: register(t0);
SamplerState basicSampler	: register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	float2 SunPos = float2(1.f, 0.f);
	float InitDecay = 0.2f;
	float DistDecay = 0.8f;
	float3 RayColor = float3(1, 1, 1);

	float rayIntensity = OcclusionTex.Sample(basicSampler, input.uv).r;
	float2 dirToSun = SunPos - input.uv;
	float2 rayDelta = dirToSun / 7.f;
	float2 rayOffset = dirToSun;

	float decay = InitDecay;
	for (int i = 0; i < 8; ++i)
	{
		float fCurIntensity = OcclusionTex.Sample(basicSampler, input.uv + rayOffset).r;
		rayIntensity += fCurIntensity * decay;
		decay += DistDecay;
		rayOffset += rayDelta;
	}

	return float4(rayIntensity.rrr, 1.f);
}