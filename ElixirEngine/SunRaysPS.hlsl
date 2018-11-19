struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

Texture2D OcclusionTex		: register(t0);
SamplerState basicSampler	: register(s0);

cbuffer externalData
{
	float2 SunPos;
};

static const int NUM_STEPS = 64;
static const float NUM_DELTA = 1.0 / 63.0f;

float4 main(VertexToPixel input) : SV_TARGET
{
	float MaxDeltaLen = 0.005f;
	float InitDecay = 0.2f;
	float DistDecay = 0.8f;
	float3 RayColor = float3(1, 1, 1);

	float2 dirToSun = (SunPos - input.uv);
	float lengthToSun = length(dirToSun);
	dirToSun /= lengthToSun;

	float deltaLen = min(MaxDeltaLen, lengthToSun * NUM_DELTA);
	float2 rayDelta = dirToSun * deltaLen;

	float stepDecay = DistDecay * deltaLen;

	float2 rayOffset = float2(0.0, 0.0);
	float decay = InitDecay;
	float rayIntensity = 0.0f;

	for (int i = 0; i < NUM_STEPS; ++i)
	{
		float fCurIntensity = OcclusionTex.Sample(basicSampler, input.uv + rayOffset).r;
		rayIntensity += fCurIntensity * decay;
		rayOffset += rayDelta;
		decay = saturate(decay - stepDecay);
	}

	return float4(rayIntensity, 0.0, 0.0, 0.0);
}