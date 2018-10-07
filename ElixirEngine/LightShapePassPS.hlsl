#define POINT_INTENSITY 0.5

struct DirectionalLight
{
	float4 AmbientColor;
	float4 DiffuseColor;
	float3 Direction;
	float Padding;
};

struct SpotLight
{
	float4 Color;
	float4 Direction;
	float3 Position;
	float Range;
	float SpotlightAngle;
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

float Attenuate(float3 lightPosition, float lightRange, float3 worldPos)
{
	float dist = distance(lightPosition, worldPos);

	// Ranged-based attenuation
	float att = saturate(1.0f - (dist * dist / (lightRange * lightRange)));

	// Soft falloff
	return att * att;
}

float4 calculatePointLight(float3 normal, float3 worldPos, PointLight light)
{
	float3 dirToPointLight = normalize(light.Position - worldPos);
	float distance = length(worldPos - light.Position);
	float pointNdotL = dot(normal, dirToPointLight);
	pointNdotL = saturate(pointNdotL) * POINT_INTENSITY;
	float attenuation = Attenuate(light.Position, light.Range, worldPos);
	//float attenuation = max(1.0f / (distance * distance) - 1.0f/(light.Range * light.Range), 0.0f);
	return (light.Color * pointNdotL);// *attenuation;// *attenuation;// attenuation;
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
	int3 sampleIndices = int3(pIn.position.xy, 0);
	float3 albedo = gAlbedoTexture.Load(sampleIndices).rgb;
	float3 normal = gNormalTexture.Load(sampleIndices).rgb;
	float3 worldPos = gWorldPosTexture.Load(sampleIndices).rgb;
	float3 pointLightValue = calculatePointLight(normal, worldPos, pointLight).rgb;
	float3 finalColor = pointLightValue * albedo;
	return float4(finalColor,1.0f);
}