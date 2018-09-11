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
	DirectionalLight light;
	light.AmbientColor = float4(0.1f, 0.1f, 0.1f, 0);
	light.Direction = float3(1, 0, 0);
	light.DiffuseColor = float4(1.0f, 0.1f, 0.1f, 0);
	float4 lightColor = calculateDirectionalLight(input.normal, light);
	return lightColor * t1.Sample(s1, input.uv);
}