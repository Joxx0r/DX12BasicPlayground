
#include "common.hlsl"

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

float3 GetPosition(float2 uv)
{
	float z = gDepthMap.Sample(gsamPointWrap, uv).r;
	
	float3 pos;
	pos.x = lerp(gFrustumCornerOne.x, gFrustumCornerThree.x, uv.x);
	pos.y = lerp(gFrustumCornerTwo.y, gFrustumCornerOne.y, uv.y); 

	return(z * float3(pos.x, pos.y, gFrustumCornerTwo.z));
}

float3 SchlickFresnel(float3 R0, float3 normal, float3 lightVec)
{
    float cosIncidentAngle = saturate(dot(normal, lightVec));

    float f0 = 1.0f - cosIncidentAngle;
    float3 reflectPercent = R0 + (1.0f - R0)*(f0*f0*f0*f0*f0);

    return reflectPercent;
}

float3 Fresnel(float3 substance,float3 l,float3 h)
{
    return(substance+(1-substance)*pow(2, -5.55473*dot(l,h)-6.98316*dot(l,h)));
}

float4 PS(VertexOut pin) : SV_Target	
{
	float t = pin.PosH.x / gWindowWidth;
	float y = pin.PosH.y / gWindowHeight;
	float2 tex = float2(t, y);
	if(gDebugValue == 0)
	{
		return gTextureMap[0].Sample(gsamLinearWrap, tex.xy);
	}
	else
	if(gDebugValue == 1)
	{
		float3 normal = normalize(gTextureMap[1].Sample(gsamLinearWrap, tex.xy) * 2 - 1).xyz;
		return float4(abs(normal.x), abs(normal.y), abs(normal.z), 1);
	}
	else
	if(gDebugValue == 2)
	{
		return gTextureMap[2].Sample(gsamLinearWrap, tex.xy);
	}
	else
	if(gDebugValue == 3)
	{
		float value = gTextureMap[3].Sample(gsamLinearWrap, tex.xy).r;
		return float4(value, value, value, 1);
	}
	else
	if(gDebugValue == 4)
	{
		float value = gTextureMap[3].Sample(gsamLinearWrap, tex.xy).g;
		return float4(value, value, value, 1);
	}
	else
	if(gDebugValue == 5)
	{
		float value = gTextureMap[3].Sample(gsamLinearWrap, tex.xy).b;
		return float4(value, value, value, 1);
	}
	else
	if(gDebugValue == 6)
	{
		float depth = gDepthMap.Sample(gsamLinearWrap, tex.xy).r;
		return float4(depth,0,0, 1);
	}
	else
	if(gDebugValue == 7)
	{
		float3 viewScreenPosition = GetPosition(tex.xy);
		return float4(viewScreenPosition.xyz, 1);
	}
	else
	if(gDebugValue == 8)
	{
		float4 viewScreenPosition = float4(GetPosition(tex.xy).xyz, 1);
		viewScreenPosition = mul(viewScreenPosition, gInvView);
		viewScreenPosition.xyz /= viewScreenPosition.w;
		return float4(viewScreenPosition.xyz, 1);
	}
	else
	if(gDebugValue == 9)
	{
		float zOverW = gDepthMap.SampleLevel(gsamPointWrap, tex.xy, 0).r;  

		float4 projectionPosition = float4(tex.x * 2 - 1, (1 - tex.y) * 2 - 1,  
			zOverW, 1); 
		float4 WorldNonDiv= mul(projectionPosition, gInvProj);
		float4 worldPos = WorldNonDiv / WorldNonDiv.w;
		return float4(worldPos.xyz, 1);
	}
	else
	if(gDebugValue == 10)
	{
 		float3 substance = gTextureMap[2].Sample(gsamLinearWrap, tex.xy).rgb;

	    clip(substance.r == 0 ? -1 : 1);

 		//just clip stuff we cant see
 		float3 lightLocations[] = { float3(0,500,0 ), float3(0,0,500 ), float3(0,0,500 ) };

	    float3 albedo = gTextureMap[0].Sample(gsamLinearWrap, tex.xy).rgb;
	    float3 surfaceNorm = normalize(gTextureMap[1].Sample(gsamLinearWrap, tex.xy).rgb * 2 - 1);
	    float2 roughnessAO = gTextureMap[2].Sample(gsamLinearWrap, tex.xy).rg;

		float zOverW = gDepthMap.SampleLevel(gsamPointWrap, tex.xy, 0).r;  

		float4 projectionPosition = float4(tex.x * 2 - 1, (1 - tex.y) * 2 - 1,  
			zOverW, 1); 
		float4 WorldNonDiv= mul(projectionPosition, gInvViewProj);
		float3 worldPos = (WorldNonDiv / WorldNonDiv.w).xyz;

	    float3 finalColor = float3(0,0,0);
	    for(int i =0; i < 3; i++)
	    {
			float3 worldLightLoc = lightLocations[i];
			float3 toLight = worldLightLoc - worldPos;
			float3 toEye = gEyePosW - worldPos;
			
			float3 toEyeNorm = normalize(toEye);
			float3 toLightNorm = normalize(toLight);
			float3 halfV = normalize(toEyeNorm + toLightNorm);

		    float3 fresnelFactor = Fresnel(substance, halfV, toLightNorm);
			float Ap=pow(8192,roughnessAO.r);
			const float Normalization = (8 + Ap) / 8;
			const float Geometry = 4 / dot(halfV, halfV);
			const float Distribution = Normalization * pow(saturate(dot(surfaceNorm, halfV)), Ap);
			 
			float nDotL = dot(toLightNorm, surfaceNorm);
			float3 diffuseLight = nDotL * albedo * (1 - substance);
			float3 spec = fresnelFactor * Geometry * Distribution * nDotL * roughnessAO.g;

		
			finalColor += (saturate(diffuseLight.xyz +spec.xyz));
	    }

		return float4(finalColor.rgb, 1);
	}

	return float4(1,1,1,1);
}


