
struct DirectionalLight
{
	float4 AmbientColor;
	float4 DiffuseColor;
	float3 Direction;
	float Padding;
};

struct PointLight
{
	float4 Color;
	float3 Position;
	float Range;
};

cbuffer externalData : register(b0)
{
	DirectionalLight dirLight;
	PointLight pointLight;
	float4x4 invProjView;
	float3 cameraPosition;
}

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

float4 calculateDirectionalLight(float3 normal, DirectionalLight light)
{
	float3 dirToLight = normalize(-light.Direction);
	float NdotL = dot(normal, dirToLight);
	NdotL = saturate(NdotL);
	return light.DiffuseColor * NdotL + light.AmbientColor;
}

float4 calculatePointLight(float3 normal, float3 worldPos, PointLight light)
{
	float3 dirToPointLight = normalize(light.Position - worldPos);
	float pointNdotL = dot(normal, dirToPointLight);
	pointNdotL = saturate(pointNdotL);
	return light.Color * pointNdotL;
}

Texture2D gAlbedoTexture : register(t0);
Texture2D gNormalTexture : register(t1);
Texture2D gWorldPosTexture : register(t2);
Texture2D gRoughnessTexture : register(t3);
Texture2D gMetalnessTexture : register(t4);

Texture2D gLightShapePass: register(t5);
Texture2D gDepth: register(t6);

sampler basicSampler;

float4 main(VertexToPixel pIn) : SV_TARGET
{
	float3 albedo = gAlbedoTexture.Sample(basicSampler, pIn.uv).rgb;
	float3 normal = gNormalTexture.Sample(basicSampler, pIn.uv).rgb;
	float3 worldPos = gWorldPosTexture.Sample(basicSampler, pIn.uv).rgb;
	float3 otherlights = gLightShapePass.Sample(basicSampler, pIn.uv).rgb;

	float3 lightValue = calculateDirectionalLight(normal, dirLight).xyz;
	float3 finalColor = lightValue * albedo;

	return float4(otherlights + finalColor,1.0f);

}