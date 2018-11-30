
Texture2D<float> DepthTex		: register(t0);
RWTexture2D<float> OcclusionRW	: register(u0);

[numthreads(8, 8, 1)]
void main( uint3 dThreadId : SV_DispatchThreadID )
{
	float depth = DepthTex[dThreadId.xy];
	OcclusionRW[dThreadId.xy].r = depth > 0.9999f;
}