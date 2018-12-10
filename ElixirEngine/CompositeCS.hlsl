//Simply adds two textures and stores it in output UAV

Texture2D<float4>	InputASRV		: register(t0);
Texture2D<float4>	InputBSRV		: register(t4);
RWTexture2D<float4>	OutputRW		: register(u0);


[numthreads(64, 1, 1)]
void main( uint3 dThreadId : SV_DispatchThreadID )
{
	float2 tex = dThreadId.xy;
	OutputRW[tex] = InputASRV[tex] + InputBSRV[tex];
}