#pragma once

#include "RevDefines.h"
#include "DirectXMath.h"
#include <string>
#include <vector>
#include "D3D12.h"
#include "MathLib\RevVector.h"
#include "MathLib\RevMatrix.h"
#include "MathLib\RevQuat.h"
#include "RevEnums.h"

using namespace DirectX;

// Simple struct to represent a material for our demos.  A production 3D engine
// would likely create a class hierarchy of Materials.
struct RevMaterial
{
	// Unique material name for lookup.
	std::string Name;

	// Index into constant buffer corresponding to this material.
	int MatCBIndex = -1;

	// Index into SRV heap for diffuse texture.
	int DiffuseSrvHeapIndex = -1;

	// Index into SRV heap for normal texture.
	int NormalSrvHeapIndex = -1;

	// Dirty flag indicating the material has changed and we need to update the constant buffer.
	// Because we have a material constant buffer for each FrameResource, we have to apply the
	// update to each FrameResource.  Thus, when we modify a material we should set 
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	int NumFramesDirty = 1;

	// Material constant buffer data used for shading.
	XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = .25f;
	XMFLOAT4X4 MatTransform;
};

struct RevModelFrameRender
{
	struct ID3D12GraphicsCommandList* m_commandList;
	class RevWorld* m_worldToRender;
	uint32_t m_currentRenderFrameResourceIndex;
	uint32_t m_amountToRender;
	uint32_t m_startIndex;
	uint32_t m_width;
	uint32_t m_height;
};

enum class RevEditorActions : uint8_t
{
	SaveModel,
	MAX
};

#define REV_MAX_INFLUENCES_PER_BONE 4

struct RevBone
{
	char m_name[512];
	RevMatrix m_offsetMatrix;
	unsigned char m_parentIndex;
	uint32_t m_channelIndex;
};


struct RevModelData
{
	// System memory copies.  Use Blobs because the vertex/index format can be generic.
	// It is up to the client to cast appropriately.  
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;
	std::vector<RevBone> m_bones;

	ID3DBlob* m_vertexBufferCPUMemory = nullptr;
	ID3DBlob* m_indexBufferCPUMemory = nullptr;

	ID3D12Resource* m_vertexBufferGpu = nullptr;
	ID3D12Resource* m_indexBufferGPU = nullptr;

	ID3D12Resource* m_vertexBufferUpload = nullptr;
	ID3D12Resource* m_indexBufferUpload = nullptr;

	ID3D12Resource* m_diffuseTexture;
	ID3D12Resource* m_substanceTexture;
	ID3D12Resource* m_normalTexture;
	ID3D12Resource* m_roughnessAoEmissiveTexture;

	std::vector<struct RevAnimationInstanceData*> m_animationInstances;

	struct ID3D12RootSignature* m_rootSignature = nullptr;
	struct ID3D12PipelineState* m_pso = nullptr;
	struct RevShader* m_shader = nullptr;

	// Data about the buffers.
	UINT m_vertexStride = 0;
	UINT m_vertexBufferByteSize = 0;
	UINT m_indexBufferByteSize = 0;
	UINT m_indexCount = 0;
	UINT m_startIndexLocation = 0;
	INT m_baseVertexLocation = 0;

	DXGI_FORMAT m_indexFormat = DXGI_FORMAT_R16_UINT;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView()const;
};

struct RevFrameSnapshotData
{
	RevVector3 m_cameraLocation;
	RevVector2 m_mouseDelta;
	float m_deltaTime = 0.0f;
	float m_fps = -1.0f;
};

struct RevEngineGlobalData
{
	UINT m_amountOfModelsToSpawn = 1; 
};

struct RevDescriptorInitializationData
{
	struct RevHeapCreation
	{
		RevHeapCreation()
		{
			memset(this, 0, sizeof(RevHeapCreation));
		}

		RevHeapCreation(bool isRtv, bool isSrv, DXGI_FORMAT format, ID3D12Resource* resourceToUse = nullptr)
		{
			m_isRtv = isRtv;
			m_isCbvSrvUav = isSrv;
			m_format = format;
			m_resourceToUse = resourceToUse;
		}

		bool m_isRtv;
		bool m_isCbvSrvUav;
		DXGI_FORMAT m_format;
		struct ID3D12Resource* m_resourceToUse;
		bool m_isDsv = false;
	};

	RevHeapCreation* m_descrptionCreationData = nullptr;
	struct D3D12_CPU_DESCRIPTOR_HANDLE* m_preAllocatedSRVHeap = nullptr;
	struct D3D12_CPU_DESCRIPTOR_HANDLE* m_preAllocatedRTVHeap = nullptr;


	UINT m_numDescriptorCreation = 1;
	UINT m_additionalSRVSize = 0;
	UINT m_additionalRTVSize = 0;
	UINT m_width = 1024;
	UINT m_height = 720;

};

struct RevDescriptorRuntimeData
{
	struct ID3D12DescriptorHeap* m_srvDescriptorHeap = nullptr;
	struct ID3D12DescriptorHeap* m_rtvHeap = nullptr;
	std::vector<struct ID3D12Resource*> m_resource;

	D3D12_CPU_DESCRIPTOR_HANDLE m_scratchSrvDescriptorHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE m_scratchRtvDescriptorHeap;
};

struct RevDescriptorHeapIncrementSizeData
{
	UINT m_cbvSrvUavSize;
	UINT m_rtvSize;
	UINT m_dsvSize;
};


struct RevLight
{
	RevVector3 m_location;
	RevVector3 m_color;
	float m_radius;
	RevLightType m_type;
};

struct RevLightVertex
{
	RevVector3 m_location;
};

struct RevVertex
{
	RevVertex()
	{

	}

	RevVector3 m_location;
	RevVector2 m_tex;
	RevVector3 m_normal;
	RevVector3 m_biNormal;
	RevVector3 m_tangent;
};

struct RevAnimatedVertex : public RevVertex
{
	RevAnimatedVertex() : 
		RevVertex()
	{
		m_boneWeights[0] = 0.0f;
		m_boneWeights[1] = 0.0f;
		m_boneWeights[2] = 0.0f;
		m_boneWeights[3] = 0.0f;
		memset(m_boneInfluences, UINT8_MAX, sizeof(unsigned char) * REV_MAX_INFLUENCES_PER_BONE);
	}
	float m_boneWeights[REV_MAX_INFLUENCES_PER_BONE];
	unsigned char m_boneInfluences[REV_MAX_INFLUENCES_PER_BONE];
};

struct RevKeyFrame
{
	RevQuat m_rotation;
	RevVector3 m_location;
	RevVector3 m_scale;
	float m_time;

	void Serialize(class RevArchive& archive);
};

struct RevAnimationChannel
{
	std::vector<RevKeyFrame> m_keyFrames;
	std::string m_nodeName;
	UINT8 m_boneIndex;


	void Serialize(class RevArchive& archive);
};

struct RevAnimationUpdateData
{
	RevAnimationUpdateData(
		RevMatrix* outMatrixes,
		std::vector<struct RevBone>& outBones,
		float deltaTime)
		:m_outBoneMatrixes(outMatrixes),
		m_bones(outBones),
		m_deltaTime(deltaTime)
	{
	}
	class RevMatrix* m_outBoneMatrixes;
	std::vector<struct RevBone>& m_bones;
	float m_deltaTime;
};

struct RevAnimationInstanceData
{
	std::vector<RevAnimationChannel> m_channels;
	std::string m_name;
	float m_duration;

	void Serialize(class RevArchive& archive);
};

struct RevTextureInitializationData
{
	UINT m_nTexturePaths;
	char m_diffuseTexturePath[512];
	char m_normalTexturePath[512];
	char m_substanceTexturePath[512];
	char m_roughnessAOEmissivePath[512];

	void Serialize(class RevArchive& archive);
};

struct RevNormalModelInitializationData
{
	RevVertex* vertices;
	UINT* indices;
	UINT m_nVertexes;
	UINT m_nIndices;
	RevTextureInitializationData m_textureInitialization;

	void Serialize(class RevArchive& archive);
};

struct RevAnimatedNodelInitializationData
{
	RevAnimatedVertex* vertices;
	UINT* indices;
	RevBone* bones;

	UINT m_nBones;
	UINT m_nVertexes;
	UINT m_nIndices;

	std::vector<RevAnimationInstanceData> instanceData;
	RevTextureInitializationData m_textureInitialization;

	void Serialize(class RevArchive& archive);
};
