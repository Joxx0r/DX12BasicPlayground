#include "common.hlsl"

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld;
};

struct VertexIn
{
	float3 PosL  : POSITION;
	float2 Tex : TEX;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
	float2 TexC  : TEX;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	vout.PosH = mul(float4(vin.PosL.xyz, 1), gWorld);
	vout.TexC = vin.Tex;
    return vout;
}
