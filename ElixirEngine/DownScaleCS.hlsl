
Texture2D			input	: register(t0);
RWTexture2D<float4> output	: register(u0);

float4 DownScale4x4(uint2 CurPixel)
{
	uint2 Res = float2(1920 / 4, 1080 / 4);
	float4 downScaled = float4(0.0, 0.0, 0.0, 0.0);
	// Skip out of bound pixels
	if (CurPixel.y < Res.y)
	{
		// Sum a group of 4x4 pixels
		int3 nFullResPos = int3(CurPixel * 4, 0);
		[unroll]
		for (int i = 0; i < 4; i++)
		{
			[unroll]
			for (int j = 0; j < 4; j++)
			{
				downScaled += input.Load(nFullResPos, int2(j, i));
			}
		}

		downScaled /= 16.0;
	}
	GroupMemoryBarrierWithGroupSync(); // Sync before next step
	downScaled = saturate(downScaled);
	return downScaled;
}


[numthreads(8, 8, 1)]
void main(uint3 dID : SV_DispatchThreadID)
{
	output[dID.xy] = DownScale4x4(dID.xy);
}