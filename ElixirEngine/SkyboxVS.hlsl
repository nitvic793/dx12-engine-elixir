struct VertexInput
{
	float3 pos		: POSITION;
	float2 uv		: TEXCOORD;
	float3 normal	: NORMAL;
	float3 tangent	: TANGENT;
};

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 uvw			: TEXCOORD;
};

cbuffer ConstantBuffer : register(b0)
{
	float4x4 worldViewProjection;
	float4x4 world;
	float4x4 view;
	float4x4 projection;
};

VertexToPixel main(VertexInput input)
{
	// Set up output
	VertexToPixel output;

	// Make a view matrix with NO translation
	matrix viewNoMovement = view;
	viewNoMovement._41 = 0;
	viewNoMovement._42 = 0;
	viewNoMovement._43 = 0;

	// Calculate output position
	matrix viewProj = mul(viewNoMovement, projection);
	output.position = mul(float4(input.pos, 1.0f), viewProj);

	// Ensure our polygons are at max depth
	output.position.z = output.position.w;

	// Use the cube's vertex position as a direction in space
	// from the origin (center of the cube)
	output.uvw = input.pos;

	return output;
}