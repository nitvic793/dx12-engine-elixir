struct VertexOutput
{
	float4 pos: SV_POSITION;
	float4 color: COLOR;
};

float4 main(VertexOutput input) : SV_TARGET
{
	// return interpolated color
	return input.color;
}