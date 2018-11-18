
Texture2D<float> OcclusionTex : register(t0);
RWTexture2D<float4> OutTexture : register(u0);

[numthreads(8, 8, 1)]
void main( uint3 dThreadId : SV_DispatchThreadID )
{
	float2 SunPos = float2(1280.f, 720.f);
	float InitDecay = 0.2f;
	float DistDecay = 0.8f;
	float3 RayColor = float3(1, 1, 1);
	float2 uv = dThreadId.xy;
	
	float rayIntensity = OcclusionTex[dThreadId.xy].r;
	float2 dirToSun = SunPos - uv;
	float2 rayDelta = dirToSun / 7.f;
	float2 rayOffset = dirToSun;

	float decay = InitDecay;
	for (int i = 0; i < 8; ++i)
	{
		float fCurIntensity = OcclusionTex[dThreadId.xy].r;
		rayIntensity += fCurIntensity * decay;
		decay += DistDecay;
		rayOffset += rayDelta;
	}

	OutTexture[dThreadId.xy] =  float4(rayIntensity.rrrr);
}