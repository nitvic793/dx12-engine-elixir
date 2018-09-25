struct VertexOutput
{
	float4 pos	: SV_POSITION;
	float2 uv	: TEXCOORD;
	float3 normal	: NORMAL;
	float3 tangent	: TANGENT;
	float3 worldPos	: POSITION;
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

Texture2D t1 : register(t0);
Texture2D normalTexture : register(t1);
SamplerState s1 : register(s0);

float3 calculateNormalFromMap(float2 uv, float3 normal, float3 tangent)
{
	float3 normalFromTexture = normalTexture.Sample(s1, uv).xyz;
	float3 unpackedNormal = normalFromTexture * 2.0f - 1.0f;
	float3 N = normal;
	float3 T = normalize(tangent - N * dot(tangent, N));
	float3 B = cross(N, T);
	float3x3 TBN = float3x3(T, B, N);
	return normalize(mul(unpackedNormal, TBN));
}

float4 calculateDirectionalLight(float3 normal, DirectionalLight light)
{
	float3 dirToLight = normalize(-light.Direction);
	float NdotL = dot(normal, dirToLight);
	NdotL = saturate(NdotL);
	return light.DiffuseColor * NdotL + light.AmbientColor;
}

float4 main(VertexOutput input) : SV_TARGET
{
	input.normal = normalize(input.normal);
	float4 lightColor = calculateDirectionalLight(input.normal, dirLight);
	return lightColor * t1.Sample(s1, input.uv);
}