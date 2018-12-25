
#ifndef _LIGHTING_HLSLI
#define _LIGHTING_HLSLI


struct DirectionalLight
{
	float4 AmbientColor;
	float4 DiffuseColor;
	float3 Direction;
	float Intensity;
};

struct PointLight
{
	float4 Color;
	float3 Position;
	float Range;
};

static const int MaxPointLights = 16;

float Attenuate(float3 position, float range, float3 worldPos)
{
	float dist = distance(position, worldPos);
	float att = saturate(1.0f - (dist * dist / (range * range)));
	return att * att;
}

static const float MIN_ROUGHNESS = 0.0000001f; 
static const float F0_NON_METAL = 0.04f;
static const float PI = 3.14159265359f;


float DiffusePBR(float3 normal, float3 dirToLight)
{
	return saturate(dot(normal, dirToLight));// Lambert diffuse 
}

// GGX (Trowbridge-Reitz)
float SpecDistribution(float3 n, float3 h, float roughness)
{
	float NdotH = saturate(dot(n, h));
	float NdotH2 = NdotH * NdotH;
	float a = roughness * roughness;
	float a2 = max(a * a, MIN_ROUGHNESS); 

	// ((n dot h)^2 * (a^2 - 1) + 1)
	float denomToSquare = NdotH2 * (a2 - 1) + 1;

	return a2 / (PI * denomToSquare * denomToSquare);
}


// Fresnel term - Schlick approx.
float3 Fresnel(float3 v, float3 h, float3 f0)
{
	float VdotH = saturate(dot(v, h));
	return f0 + (1 - f0) * pow(1 - VdotH, 5);
}

float3 FresnelSchlickRoughness(float3 v, float3 h, float f0, float roughness)
{
	float VdotH = saturate(dot(v, h));
	float r1 = 1.0f - roughness;
	return f0 + (max(float3(r1, r1, r1), f0) - f0) * pow(1 - VdotH, 5.0f);
}


//Schlick-GGX 
float GeometricShadowing(float3 n, float3 v, float3 h, float roughness)
{
	// End result of remapping:
	float k = pow(roughness + 1, 2) / 8.0f;
	float NdotV = saturate(dot(n, v));

	// Final value
	return NdotV / (NdotV * (1 - k) + k);
}


float3 MicrofacetBRDF(float3 n, float3 l, float3 v, float roughness, float metalness, float3 specColor, out float3 kS)
{
	float3 h = normalize(v + l);

	float D = SpecDistribution(n, h, roughness);
	float3 F = Fresnel(v, h, specColor); 
	float G = GeometricShadowing(n, v, h, roughness) * GeometricShadowing(n, l, h, roughness);
	kS = F;
	return (D * F * G) / (4 * max(dot(n, v), dot(n, l)));
}


float3 DiffuseEnergyConserve(float diffuse, float3 specular, float metalness)
{
	return diffuse * ((1 - saturate(specular)) * (1 - metalness));
}

float3 AmbientPBR(float3 kD, float metalness, float3 diffuse, float ao, float3 specular)
{
	kD *= (1.0f - metalness);
	return (kD * diffuse + specular) * ao;
}

float3 DirLightPBR(DirectionalLight light, float3 normal, float3 worldPos, 
	float3 camPos, float roughness, float metalness, 
	float3 surfaceColor, float3 specularColor, float3 irradiance, float3 prefilteredColor, float2 brdf, float shadowAmount)
{
	float ao = 1.0f;
	float3 toLight = normalize(-light.Direction);
	float3 toCam = normalize(camPos - worldPos);
	float diff = DiffusePBR(normal, toLight);
	float3 kS = float3(0.f, 0.f, 0.f);
	float3 spec = MicrofacetBRDF(normal, toLight, toCam, roughness, metalness, specularColor, kS);
	float3 balancedDiff = DiffuseEnergyConserve(diff, spec, metalness);
	float3 specular = prefilteredColor * (kS * brdf.x + brdf.y);
	float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
	float3 diffuse = irradiance * surfaceColor;
	float3 ambient = AmbientPBR(kD, metalness, diffuse, ao, specular);

	return (balancedDiff * surfaceColor + spec) * light.DiffuseColor.rgb * light.Intensity * shadowAmount + ambient;
}

float3 PointLightPBR(PointLight light, float3 normal, float3 worldPos, float3 camPos, float roughness, float metalness, float3 surfaceColor, float3 specularColor, float3 irradiance)
{
	float ao = 1.0f;
	float3 toLight = normalize(light.Position - worldPos);
	float3 toCam = normalize(camPos - worldPos);
	float atten = Attenuate(light.Position, light.Range/1.8f, worldPos);
	float diff = DiffusePBR(normal, toLight);
	float3 kS = float3(0.f, 0.f, 0.f);
	float3 spec = MicrofacetBRDF(normal, toLight, toCam, roughness, metalness, specularColor, kS);

	float3 balancedDiff = DiffuseEnergyConserve(diff, spec, metalness);

	return (balancedDiff * surfaceColor + spec) * atten * 1 /*light.Intensity*/ * light.Color.rgb;// +ambient;
}



#endif 