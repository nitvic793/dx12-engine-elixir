struct GSOutput
{
	float4 pos : SV_POSITION;
};

cbuffer	cbuffercbShadowMapCubeGS : register(b1)
{
	float4x4 CubeViewProj[6];
}

struct GeometryOutput
{
	float4	Pos		: SV_POSITION;
	uint	RTIndex	: SV_RenderTargetArrayIndex;
};

[maxvertexcount(18)]
void main(triangle float4 InPos[3] : SV_Position, inout	TriangleStream<GeometryOutput> OutStream)
{
	for (int iFace = 0; iFace < 6; iFace++)
	{
		GeometryOutput output;
		output.RTIndex = iFace;
		for (int v = 0; v < 3; v++) {
			output.Pos = mul(InPos[v], CubeViewProj[iFace]);
			OutStream.Append(output);
		}
		OutStream.RestartStrip();
	}
}