
#include "common.hlsl"


float SSAO_scale=0.3f;
float SSAO_sample_rad=0.8*1.41f;
float SSAO_bias=0.05f;
float SSAO_Intensity=8.0f;
float SSAO_SelfOcclusion=0.0f;

struct VertexIn
{	
	float2 PosL  : POSITION;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	vout.PosH = float4(vin.PosL.xy, 0, 1);
    return vout;
}

float3 GetPosition(float2 tex)
{
	float zOverW = gDepthMap.SampleLevel(gsamPointWrap, tex.xy, 0).r;  

	float4 projectionPosition = float4(tex.x * 2 - 1, (1 - tex.y) * 2 - 1,  
		zOverW, 1); 
	float4 WorldNonDiv= mul(projectionPosition, gInvViewProj);
	return (WorldNonDiv / WorldNonDiv.w).xyz;
}

float2 GetRandom(float2 uv)
{
//Where “g_screen_size” contains the width and height of the screen in pixels and “random_size” is the size of the random texture 
	return normalize(RandomTexture.Sample(samplePoint, float2(gWindowWidth, gWindowHeight) * uv / float2(64,64)).xy * 2.0f - 1.0f);
}

float doAmbientOcclusion(in float2 tcoord,in float2 uv, in float3 p, in float3 cnorm)
{
	float3 diff = GetPosition(tcoord + uv) - p;
	const float3 v = normalize(diff);
	const float d = length(diff)*0.3;
	float val = max(0.0,dot(cnorm,v) - SSAO_bias)*(1.0/(1.0+d));
	return val;
}



float4 PS(VertexOut pin) : SV_Target	
{
	float t = pin.PosH.x / gWindowWidth;
	float y = pin.PosH.y / gWindowHeight;
	float2 tex = float2(t, y);
	float3 p = GetPosition( tex );

 	const float2 vec2[8] = {float2(1,1),
						float2(1,-1),
                         float2(-1,1),
						 float2(-1,-1),
                         float2(0,1),float2(0,-1),
                         float2(-1,0),float2(1,0)
                       };

	float3 n = normalize(gTextureMap[1].Sample(gsamLinearWrap, tex.xy) * 2 - 1).xyz;
	float2 rand = GetRandom(tex);

	float ao = 0.0f;
	float rad = 0.3/(p.z);
	rad= min(rad,0.03);

	int iterations = 8;
	for (int j = 0; j < iterations; ++j)
	{
	  float2 coord1 = reflect(vec2[j],rand)*rad;
	  float2 coord2 = float2(coord1.x*0.707 - coord1.y*0.707,
				  coord1.x*0.707 + coord1.y*0.707);
	  
	  ao += doAmbientOcclusion(tex,coord1*0.25, p, n);
	  ao += doAmbientOcclusion(tex,coord2*0.5, p, n);
	  ao += doAmbientOcclusion(tex,coord1*0.75, p, n);
	  ao += doAmbientOcclusion(tex,coord2, p, n);
	}
	ao/=(float)iterations;
	
	//Do stuff here with your occlusion value “ao”: modulate ambient lighting, write it to a buffer for later //use, etc.
	ao=saturate(1.0-ao);
	return float4(ao,0,0,1);

}


