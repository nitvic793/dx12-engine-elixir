#define threshold 0.04

Texture2D<float>	DepthTex	: register(t0);
Texture2D<float4>	Pixels		: register(t4);

RWTexture2D<float4> OutputRW	: register(u0);

[numthreads(8, 8, 1)]
void main(uint3 dThreadId : SV_DispatchThreadID)
{
	int2 offsets[4] = {
		int2(-1,0),
		int2(0,-1),
		int2(1,0),
		int2(0,1)
	};

	float depth = DepthTex[dThreadId.xy];
	float3 color = Pixels[dThreadId.xy].rgb;
	float4 edges = {
		DepthTex[dThreadId.xy + offsets[0]],
		DepthTex[dThreadId.xy + offsets[1]],
		DepthTex[dThreadId.xy + offsets[2]],
		DepthTex[dThreadId.xy + offsets[3]]
	};

	float4 delta = abs(depth.xxxx - edges);
	float4 fEdges = step(threshold / 10.f, delta);
	float edgeVal = (1.f - dot(fEdges, 1.0) == 0.0);
	float3 edgeColor = edgeVal.rrr;
	float4 result = float4(color + edgeColor, 1.f);
	OutputRW[dThreadId.xy] = result;
}