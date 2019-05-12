
Texture2D gTextureMap[4] 	: register(t0);
Texture2D gDepthMap  		: register(t4);
Texture2D gSsaoMap  		: register(t5);

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

cbuffer cbPass : register(b1)
{
	float4x4 gView;
	float4x4 gInvView;
	float4x4 gProj;
	float4x4 gInvProj;
	float4x4 gViewProj;
	float4x4 gInvViewProj;
	float3 gEyePosW;
	float cbPerObjectPad1;
	float gNearZ;
	float gFarZ;
	float gTotalTime;
	float gDeltaTime;
	float gWindowWidth;
	float gWindowHeight;
	int gDebugValue;
	float pada;
	float padb;
	float3 gFrustumCornerOne;
	float pad1;
	float3 gFrustumCornerTwo;
	float pad2;
	float3 gFrustumCornerThree;
	float pad3;
	float3 gFrustumCornerFour;
};


float3 FindViewPositionFromNDC(float3 ndc)
{
	float4 viewPos = mul(float4(ndc, 1), gInvProj);
	return float3(viewPos.xyz /= viewPos.w);
}

float3 FindWorldPositionFromNDC(float3 ndc)
{
	float4 worldPos = mul(float4(ndc, 1), gInvViewProj);
	return float3(worldPos.xyz /= worldPos.w);
}