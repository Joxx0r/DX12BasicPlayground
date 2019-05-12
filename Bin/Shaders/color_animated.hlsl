

#include "common.hlsl"

#define MAX_BONES 128

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld; 
	float4x4 gBones[MAX_BONES];
};

struct VertexIn
{
	float3 PosL  : POSITION;
	float2 Tex : TEX;
	float3 Normal : NORMAL;
	float3 BiNormal : BINORMAL;
	float3 Tangent : TANGENT;
	float4 Weights : BONEINFLUENCE;
	uint4 BonesAffecting : BONEAFFECT;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
	float2 Tex : TEX;
	float3 Normal : NORMAL;
	float3 BiNormal : BINORMAL;
	float3 Tangent : TANGENT;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    weights[0] = vin.Weights.x;
    weights[1] = vin.Weights.y;
    weights[2] = vin.Weights.z;
    weights[3] = vin.Weights.w;

    float3 posL = float3(0.0f, 0.0f, 0.0f);
    float3 normalL = float3(0.0f, 0.0f, 0.0f);
    float3 tangentL = float3(0.0f, 0.0f, 0.0f);
    float3 biNormal = float3(0.0f, 0.0f, 0.0f);
    for(int i = 0; i < 4; ++i)
    {
        // Assume no nonuniform scaling when transforming normals, so 
        // that we do not have to use the inverse-transpose.

        posL += weights[i] * mul(float4(vin.PosL, 1.0f), gBones[vin.BonesAffecting[i]]).xyz;
        normalL += weights[i] * mul(vin.Normal, (float3x3)gBones[vin.BonesAffecting[i]]);
        biNormal += weights[i] * mul(vin.BiNormal.xyz, (float3x3)gBones[vin.BonesAffecting[i]]);
        tangentL += weights[i] * mul(vin.Tangent.xyz, (float3x3)gBones[vin.BonesAffecting[i]]);
    }

	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(posL.xyz, 1.0f), gWorld);
	vout.PosH = mul(vout.PosH, gViewProj);
	vout.Normal = mul(float4(normalL, 0), gWorld).xyz;
	vout.BiNormal = mul(float4(biNormal, 0), gWorld).xyz;
	vout.Tangent = mul(float4(tangentL,0) , gWorld).xyz;
	vout.Tex = vin.Tex;
    return vout;
}

struct PixelOut
{
	float4 Albedo : COLOR0;
	float4 Normal : COLOR1;
	float4 Substance : COLOR2;
	float4 RoughnessAoEmissive : COLOR3;
};

PixelOut PS(VertexOut pin) : SV_Target	
{
	PixelOut pixelOut;
	pixelOut.Albedo = float4(gTextureMap[0].Sample(gsamAnisotropicWrap, pin.Tex));
	clip(pixelOut.Albedo.a < 0.1f ? -1 : 1);

	float3x3 TBN = float3x3(
        normalize(pin.Tangent),
        normalize(pin.BiNormal),
        normalize(pin.Normal)
    );	
	float3 normal = normalize(gTextureMap[1].Sample(gsamLinearWrap, pin.Tex).xyz * 2 - 1);
	pixelOut.Normal = float4(mul(normal.xyz,TBN), 1);
	pixelOut.Normal = (pixelOut.Normal + 1) / 2;
	pixelOut.Substance = float4(gTextureMap[2].Sample(gsamLinearWrap, pin.Tex));
	pixelOut.RoughnessAoEmissive = float4(gTextureMap[3].Sample(gsamLinearWrap, pin.Tex));
	return pixelOut;
}


