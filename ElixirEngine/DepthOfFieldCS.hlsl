
Texture2D blurTexture				: register(t0);
Texture2D sharpTexture				: register(t4);
RWTexture2D<float4> resultTexture	: register(u0);

#define N 16

[numthreads(8, 8, 1)]
void main( uint3 dThreadID : SV_DispatchThreadID )
{
	float focusPlane = 6.f;
	float scale = 0.2f;
	float4 sharp = sharpTexture[dThreadID.xy];
	float3 blur = blurTexture[dThreadID.xy].rgb;
	float linearZ = sharp.a;
	float radius = (linearZ - focusPlane) * scale;
	radius = clamp(radius * 2.0, -1.0, 1.0);
	float3 result = lerp(sharp.rgb, blur, abs(radius));
	resultTexture[dThreadID.xy] = float4(result, 1.f);
}