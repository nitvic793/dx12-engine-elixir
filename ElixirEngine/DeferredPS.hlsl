struct VertexOutput
{
	float4 pos	: SV_POSITION;
	float2 uv	: TEXCOORD;
	float3 normal	: NORMAL;
	float3 tangent	: TANGENT;
	float3 worldPos	: POSITION;
};

struct PixelOutput
{
	float3 albedo: SV_TARGET0;
	float4 normal: SV_TARGET1;
	float4 worldPos: SV_TARGET2;
	float4 roughness: SV_TARGET3;
	float4 metalness: SV_TARGET4;
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

Texture2D AlbedoTexture			: register(t0);
Texture2D NormalTexture			: register(t1);
Texture2D RoughnessTexture		: register(t2);
Texture2D MetalTexture			: register(t3);
SamplerState Sampler			: register(s0);

float4 calculateDirectionalLight(float3 normal, DirectionalLight light)
{
	float3 dirToLight = normalize(-light.Direction);
	float NdotL = dot(normal, dirToLight);
	NdotL = saturate(NdotL);
	return light.DiffuseColor * NdotL + light.AmbientColor;
}

float3 calculateNormalFromMap(float2 uv, float3 normal, float3 tangent)
{
	float3 normalFromTexture = NormalTexture.Sample(Sampler, uv).xyz;
	float3 unpackedNormal = normalFromTexture * 2.0f - 1.0f;
	float3 N = normal;
	float3 T = normalize(tangent - N * dot(tangent, N));
	float3 B = cross(N, T);
	float3x3 TBN = float3x3(T, B, N);
	return normalize(mul(unpackedNormal, TBN));
}


PixelOutput main(VertexOutput input)// : SV_TARGET
{
	float3 normal = calculateNormalFromMap(input.uv, input.normal, input.tangent);
	PixelOutput output;
	output.albedo = AlbedoTexture.Sample(Sampler, input.uv).rgb;
	output.normal = float4(normalize(normal), 1.0f);
	output.worldPos = float4(input.worldPos, 0.0f);
	float roughness = RoughnessTexture.Sample(Sampler, input.uv).r;
	float metal = MetalTexture.Sample(Sampler, input.uv).r;
	output.roughness = float4(roughness, 0, 0, 0);
	output.metalness = float4(metal, 0.f, 0.f, 0);
	return output;
}