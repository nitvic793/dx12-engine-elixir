
cbuffer externalData : register(b0)
{
	int reduceFactor;
}

Texture2D inputTexture :			register(t0);
RWTexture2D<float4> outputTexture : register(u0);
SamplerState basicSampler:			register(s0);

[numthreads(8, 8, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
	outputTexture[threadID.xy] = inputTexture[threadID.xy] / (float)reduceFactor;
}