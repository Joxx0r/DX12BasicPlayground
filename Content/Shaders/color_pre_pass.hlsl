

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
};

float4 VS(VertexIn vin)
{
	float4 outPosition;
	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorld);
	vout.PosH = mul(vout.PosH, gViewProj);
    return vout;
}

