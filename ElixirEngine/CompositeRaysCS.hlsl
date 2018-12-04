
Texture2D<float4> LightRaysTex 	: register(t0);
Texture2D<float4> Pixels 		: register(t4);
RWTexture2D<float4> OutputRW		: register(u0);

cbuffer data
{
	float3 RayColor;
};

[numthreads(64, 1, 1)]
void main( uint3 dThreadId : SV_DispatchThreadID )
{
	float3 rayColor = RayColor;
	float rayIntensity = LightRaysTex[dThreadId.xy].r;
	//rayIntensity = min(0.5f, rayIntensity);
	rayIntensity = rayIntensity/10.f;
	float3 pixel = Pixels[dThreadId.xy].rgb;
	float3 rayWithColor = rayColor * rayIntensity;
	OutputRW[dThreadId.xy] = float4(rayWithColor + pixel, 1.f);
}