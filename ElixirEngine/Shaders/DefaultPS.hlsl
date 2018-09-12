struct VertexOutput
{
	float4 pos	: SV_POSITION;
	float2 uv	: TEXCOORD;
	float3 normal	: NORMAL;
	float3 tangent	: TANGENT;
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

float4 main(VertexOutput input) : SV_TARGET
{
	input.normal = normalize(input.normal);
	float4 lightColor = calculateDirectionalLight(input.normal, dirLight);
	return lightColor * t1.Sample(s1, input.uv);
}