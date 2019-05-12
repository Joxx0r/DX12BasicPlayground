

#include "common.hlsl"

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld;
	float gScale : SCALE;
};

struct VertexIn
{
	float3 PosL  : POSITION;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
	float3 WorldPosition : POSITION;
};
	
VertexOut VS(VertexIn vin)
{
	VertexOut vsOut = (VertexOut)0;
	vsOut.PosH = float4(vin.PosL * gScale, 1);
	vsOut.PosH = mul(vsOut.PosH , gWorld);
	vsOut.WorldPosition = float3(gWorld[3][0],gWorld[3][1], gWorld[3][2]);
	vsOut.PosH = mul(vsOut.PosH, gViewProj);
    return vsOut;
}

float4 PS(VertexOut pin) : SV_Target	
{
	return float4(1,1,1,1);
}


