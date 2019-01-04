
Texture2D blurTexture				: register(t0);
Texture2D sharpTexture				: register(t4);
RWTexture2D<float4> resultTexture	: register(u0);

cbuffer dofConstants: register(b0)
{
	float focusPlaneZ;
	float zScale;
}

#define N 16

[numthreads(8, 8, 1)]
void main(uint3 dThreadID : SV_DispatchThreadID)
{
	
	float focusPlane = 6.f;
	float scale = zScale;
	float4 sharp = sharpTexture[dThreadID.xy];
	float3 blur = blurTexture[dThreadID.xy / 4].rgb; //downscaled blur texture sampled
	float linearZ = sharp.a; // packed linear z
	float radius = (focusPlaneZ  - linearZ) * scale;
	radius = clamp(radius * 2.0, -1.0, 1.0);
	float3 result = lerp(sharp.rgb, blur, abs(radius));
	resultTexture[dThreadID.xy] = float4(result, 1.f);
}