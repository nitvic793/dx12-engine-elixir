struct VertexOutput
{
	float4 pos	: SV_POSITION;
	float2 uv	: TEXCOORD;
};

Texture2D t1 : register(t0);
SamplerState s1 : register(s0);

float4 main(VertexOutput input) : SV_TARGET
{
	return t1.Sample(s1, input.uv);
}