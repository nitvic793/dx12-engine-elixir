#include "Lighting.hlsli"

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

//G-Buffer
Texture2D gAlbedoTexture			: register(t0);
Texture2D gNormalTexture			: register(t1);
Texture2D gWorldPosTexture			: register(t2);
Texture2D gRoughnessTexture			: register(t3);
Texture2D gMetalnessTexture			: register(t4);
Texture2D gLightShapePass			: register(t5);
Texture2D gDepth					: register(t7); //t6 reserved for this shaders output

													//IBL
TextureCube skyIrradianceTexture	: register(t8);
Texture2D	brdfLUTTexture			: register(t9);
TextureCube skyPrefilterTexture		: register(t10);

//Shadow Dir Light
Texture2D gShadowMap				: register(t11);
Texture2D gShadowPos				: register(t12);

SamplerState			basicSampler	: register(s0);
SamplerComparisonState	shadowSampler	: register(s1);

float3 PrefilteredColor(float3 viewDir, float3 normal, float roughness)
{
	const float MAX_REF_LOD = 8.0f; // (mip level - 1) Mip Levels = 9
	float3 R = reflect(-viewDir, normal);
	return skyPrefilterTexture.SampleLevel(basicSampler, R, roughness * MAX_REF_LOD).rgb;
}

float2 BrdfLUT(float3 normal, float3 viewDir, float roughness)
{
	float NdotV = dot(normal, viewDir);
	NdotV = max(NdotV, 0.0f);
	float2 uv = float2(NdotV, roughness);
	return brdfLUTTexture.Sample(basicSampler, uv).rg;
}

float ShadowAmount(float4 shadowPos)
{
	float2 shadowUV = shadowPos.xy / shadowPos.w * 0.5f + 0.5f;
	shadowUV.y = 1.0f - shadowUV.y;
	float depthFromLight = shadowPos.z / shadowPos.w;
	float shadowAmount = gShadowMap.SampleCmpLevelZero(shadowSampler, shadowUV, depthFromLight);
	return shadowAmount;
}

float4 main(VertexToPixel pIn) : SV_TARGET
{
	int3 sampleIndices = int3(pIn.position.xy, 0);
	float4 packedAlbedo = gAlbedoTexture.Sample(basicSampler, pIn.uv);
	float3 albedo = packedAlbedo.rgb;
	float3 normal = gNormalTexture.Sample(basicSampler, pIn.uv).rgb;
	float3 worldPos = gWorldPosTexture.Sample(basicSampler, pIn.uv).rgb;
	float roughness = gRoughnessTexture.Sample(basicSampler, pIn.uv).r;
	float metal = gMetalnessTexture.Sample(basicSampler, pIn.uv).r;
	float shadowAmount = 1.f;

	float3 viewDir = normalize(cameraPosition - worldPos);
	float3 prefilter = PrefilteredColor(viewDir, normal, roughness);
	float2 brdf = BrdfLUT(normal, viewDir, roughness);
	float3 otherlights = gLightShapePass.Sample(basicSampler, pIn.uv).rgb;

	float3 specColor = lerp(F0_NON_METAL.rrr, albedo.rgb, metal);
	float3 irradiance = skyIrradianceTexture.Sample(basicSampler, normal).rgb;

	float3 finalColor = AmbientPBR(dirLight[0], normalize(normal), worldPos,
		cameraPosition, roughness, metal, albedo,
		specColor, irradiance, prefilter, brdf, shadowAmount);
	float3 totalColor = finalColor + otherlights;
	totalColor = totalColor / (totalColor + float3(1.f, 1.f, 1.f));
	totalColor = saturate(totalColor);
	float3 gammaCorrect = lerp(totalColor, pow(totalColor, 1.0 / 2.2), 0.4f);
	return float4(gammaCorrect, packedAlbedo.a);

}