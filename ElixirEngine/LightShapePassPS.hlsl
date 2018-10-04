float G_Smith(float roughness, float NoV, float NoL)
{
	float  k = (roughness + 1) * (roughness + 1) / 8;
	return  (NoV / (NoV * (1 - k) + k)) *  (NoL / (NoL * (1 - k) + k));
}

float4 GGXBRDF(float3 lightDir, float3 lightPos, float3 albedo, float3 normal, float3 viewDir, float3 specular, float gloss)
{
	const  float pi = 3.14159;
	float3 h = normalize(viewDir + lightDir);


	float NdotL = max(0, dot(normal, lightDir));
	float NdotH = max(0, dot(normal, h));
	float LdotH = max(0, dot(lightDir, h));
	float VdotH = max(0, dot(viewDir, h));
	float NdotV = max(0, dot(normal, viewDir));
	float roughness = gloss;

	//D
	float alpha = roughness * roughness;
	float alphaSqr = alpha * alpha;
	float denom = ((NdotH * NdotH) * (alphaSqr - 1.0f) + 1.0f);
	float D = alphaSqr / (pi * denom* denom);
	float FV;

	//fersnel & V
	float F_b = pow((1 - VdotH), 5);
	float F_a = 1;
	float k = alpha / 2;
	float	vis = G_Smith(roughness, NdotV, NdotL);
	float2 FV_helper = float2(F_a*vis, F_b*vis);
	float3 col = specular * FV_helper.x + (1 - specular)*FV_helper.y;

	col = (NdotL*D*col + NdotL * albedo).rgb;

	return float4(col, 1);
}

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
	pointNdotL = saturate(pointNdotL);
	//Attenuate(light.Position)
	float attenuation = max(1.0f / (distance * distance) - 1.0f/(light.Range * light.Range), 0.0f);
	return (light.Color * pointNdotL) / attenuation;
}

Texture2D gAlbedoTexture : register(t0);
Texture2D gNormalTexture : register(t1);
Texture2D gWorldPosTexture : register(t2);

Texture2D gDepth: register(t4);

sampler basicSampler;

float4 main(VertexToPixel pIn) : SV_TARGET
{
	int3 sampleIndices = int3(pIn.position.xy, 0);
	float3 albedo = gAlbedoTexture.Load(sampleIndices).rgb;
	float3 normal = gNormalTexture.Load(sampleIndices).rgb;
	float3 worldPos = gWorldPosTexture.Load(sampleIndices).rgb;
	//float3 lightValue = calculateDirectionalLight(normal, dirLight).xyz;
	float3 pointLightValue = calculatePointLight(normal, worldPos, pointLight).rgb;
	//return float4(albedo, 1.0f);
	float3 finalColor = pointLightValue * albedo;
	//finalColor += pointLightValue * albedo;
	return float4(finalColor,1.0f);

}