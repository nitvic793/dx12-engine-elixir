struct VertexOutput
{
	float4 pos	: SV_POSITION;
	float2 uv	: TEXCOORD;
	float3 normal	: NORMAL;
	float3 tangent	: TANGENT;
};

struct PixelOutput
{
	float3 albedo: SV_TARGET0;
	float4 normal: SV_TARGET1;
	float4 specGloss: SV_TARGET2;
};

struct DirectionalLight
{
	float4 AmbientColor;
	float4 DiffuseColor;
	float3 Direction;
	float Padding;
};

cbuffer externalData : register(b0)
{
	DirectionalLight dirLight;
}

float4 calculateDirectionalLight(float3 normal, DirectionalLight light)
{
	float3 dirToLight = normalize(-light.Direction);
	float NdotL = dot(normal, dirToLight);
	NdotL = saturate(NdotL);
	return light.DiffuseColor * NdotL + light.AmbientColor;
}


Texture2D t1 : register(t0);
SamplerState s1 : register(s0);

PixelOutput main(VertexOutput input) : SV_TARGET
{
	PixelOutput output;
	output.albedo =  t1.Sample(s1, input.uv);
	output.normal = float4(normalize(input.normal), 1.0f);
	output.specGloss = float4(0.5f, 0.5f, 0.5f, 1);;
	return output;
}