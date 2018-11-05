struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

Texture2D Pixels			: register(t0);
SamplerState basicSampler	: register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	return Pixels.Sample(basicSampler, input.uv);
}