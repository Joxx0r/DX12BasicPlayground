
#include "common.hlsl"

RWTexture2D<float4> outputTexture : register(u0);


struct Light {
	float3 pos; float sqrRadius;
	float3 color; float invSqrRadius;
};



#define BLOCK_SIZE 16
[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
void csMain(
	uint3 groupId : SV_GroupID,
	uint3 groupThreadId : SV_GroupThreadID,
	uint groupIndex : SV_GroupIndex,
	uint3 dispatchThreadId : SV_DispatchThreadID)
{
	groupshared uint minDepthInt;
	groupshared uint maxDepthInt;
	// --- globals above, function below -------
	float depth =
		depthTexture.Load(uint3(texCoord, 0)).r;
	uint depthInt = asuint(depth);
	minDepthInt = 0xFFFFFFFF;
	maxDepthInt = 0;
	GroupMemoryBarrierWithGroupSync();
	InterlockedMin(minDepthInt, depthInt);
	InterlockedMax(maxDepthInt, depthInt);
	GroupMemoryBarrierWithGroupSync();
	float minGroupDepth = asfloat(minDepthInt);
	float maxGroupDepth = asfloat(maxDepthInt);


	//Convert these corners into NDC and then convert them to view space
	float3 tileCorners[4];
	tileCorners[0] = FindViewPositionFromNDC(float3((float(minX)/SCREEN_WIDTH) * 2.0f - 1.0f, (float(minY)/SCREEN_HEIGHT) * 2.0f - 1.0f, 1.0f));
	tileCorners[1] = FindViewPositionFromNDC(float3((float(maxX)/SCREEN_WIDTH) * 2.0f - 1.0f, (float(minY)/SCREEN_HEIGHT) * 2.0f - 1.0f, 1.0f));
	tileCorners[2] =FindViewPositionFromNDC(float3((float(maxX)/SCREEN_WIDTH) * 2.0f - 1.0f, (float(maxY)/SCREEN_HEIGHT) * 2.0f - 1.0f);
	tileCorners[3] = FindViewPositionFromNDC(float3((float(minX)/SCREEN_WIDTH) * 2.0f - 1.0f, (float(maxY)/SCREEN_HEIGHT) * 2.0f - 1.0f, 1.0f));

	//Create the frustum planes by using the cross product between these points 
	float3 frustum[4];
	for(int i = 0; i < 4; i++)
		frustum[i] = CreatePlane(tileCorners[i],tileCorners[(i+1) & 3]);

	GroupMemoryBarrierWithGroupSync();

	int lightCount;
	StructuredBuffer<Light> lights;
	groupshared uint visibleLightCount = 0;
	groupshared uint visibleLightIndices[1024];
	// --- globals above, cont. function below ---
	uint threadCount = BLOCK_SIZE*BLOCK_SIZE;
	uint passCount = (lightCount + threadCount - 1) / threadCount;

	for (uint passIt = 0; passIt < passCount; ++passIt)
	{
		uint lightIndex = passIt*threadCount + groupIndex;
		// prevent overrun by clamping to a last ”null” light
		lightIndex = min(lightIndex, lightCount);
		if (intersects(lights[lightIndex], tile))
		{
			uint offset;
			InterlockedAdd(visibleLightCount, 1, offset);
			visibleLightIndices[offset] = lightIndex;
		}
	}
	GroupMemoryBarrierWithGroupSync();

}
