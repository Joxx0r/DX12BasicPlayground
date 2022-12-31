#include "stdafx.h"
#include "RevEngineMain.h"
#include "MathLib/RevMath.h"
#include "RevTypes.h"
#include "MicrosoftDDSLoader.h"
#include "RevEngineManager.h"
#include "RevUtils.h"
#include "RevModelManager.h"

#if USE_IMGUI
#include "imgui/imgui.h"
#endif

void RevEngineFunctions::CreateSRVRTVDescriptorHeap(
		const struct RevDescriptorInitializationData& initializationData, 
		struct RevDescriptorRuntimeData* out)
{

	struct RevTextureCreationCollectionData
	{
		uint32_t m_numRtvDescriptor = 0;
		uint32_t m_numSrvDescriptors = 0;

		void Calculate(const RevDescriptorInitializationData& initializationData)
		{
			m_numRtvDescriptor = 0;
			m_numSrvDescriptors = 0;
			for (uint32_t index = 0; index < initializationData.m_numDescriptorCreation; index++)
			{
				RevDescriptorInitializationData::RevHeapCreation heapCreation = initializationData.m_descrptionCreationData[index];
				if (heapCreation.m_isRtv)
				{
					m_numRtvDescriptor++;
				}
				if (heapCreation.m_isCbvSrvUav)
				{
					m_numSrvDescriptors++;
				}
			}

			m_numSrvDescriptors += initializationData.m_additionalSRVSize;
			m_numRtvDescriptor += initializationData.m_additionalRTVSize;
		}
	};

	RevTextureCreationCollectionData creationData;
	creationData.Calculate(initializationData);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;

	if (creationData.m_numSrvDescriptors > 0)
	{
		if (initializationData.m_preAllocatedSRVHeap)
		{
			srvHandle = *initializationData.m_preAllocatedSRVHeap;
		}
		else
		{
			D3D12_DESCRIPTOR_HEAP_DESC descHeapCbvSrv = {};
			descHeapCbvSrv.NumDescriptors = creationData.m_numSrvDescriptors;
			descHeapCbvSrv.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			descHeapCbvSrv.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			RevThrowIfFailed(FindDevice()->CreateDescriptorHeap(&descHeapCbvSrv, __uuidof(ID3D12DescriptorHeap), (void**)&out->m_srvDescriptorHeap));
			srvHandle = out->m_srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		}
	}

	if (creationData.m_numRtvDescriptor > 0)
	{
		if (initializationData.m_preAllocatedRTVHeap)
		{
			rtvHeapHandle = *initializationData.m_preAllocatedRTVHeap;
		}
		else
		{
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
			rtvHeapDesc.NumDescriptors = creationData.m_numRtvDescriptor;
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			rtvHeapDesc.NodeMask = 0;
			RevThrowIfFailed(FindDevice()->CreateDescriptorHeap(
				&rtvHeapDesc, IID_PPV_ARGS(&out->m_rtvHeap)));
			rtvHeapHandle = out->m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
		}
	}


	for (uint32_t index = 0; index < initializationData.m_numDescriptorCreation; index++)
	{
		RevDescriptorInitializationData::RevHeapCreation heapCreation = initializationData.m_descrptionCreationData[index];

		D3D12_RESOURCE_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Alignment = 0;
		texDesc.Width = initializationData.m_width;
		texDesc.Height = initializationData.m_height;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.Format = heapCreation.m_format;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		float normalClearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		CD3DX12_CLEAR_VALUE optClear(heapCreation.m_format, normalClearColor);
		ID3D12Resource* newResource = nullptr;
		
		if (heapCreation.m_resourceToUse)
		{
			newResource = heapCreation.m_resourceToUse;
		}
		else

		{
			RevThrowIfFailed(FindDevice()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&texDesc,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				&optClear,
				IID_PPV_ARGS(&newResource)));

		}

		REV_ASSERT(newResource);
		out->m_resource.push_back(newResource);

		if (heapCreation.m_isRtv)
		{
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Format = heapCreation.m_format;
			rtvDesc.Texture2D.MipSlice = 0;
			rtvDesc.Texture2D.PlaneSlice = 0;
			FindDevice()->CreateRenderTargetView(newResource, &rtvDesc, rtvHeapHandle);
			rtvHeapHandle.ptr += RevEngineFunctions::FindIncrementSizes()->m_rtvSize;
		}

		if (heapCreation.m_isCbvSrvUav)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Format = heapCreation.m_format;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = 1;
			FindDevice()->CreateShaderResourceView(newResource, &srvDesc, srvHandle);
			srvHandle.ptr += RevEngineFunctions::FindIncrementSizes()->m_cbvSrvUavSize;
		}
	}

	out->m_scratchRtvDescriptorHeap = rtvHeapHandle;
	out->m_scratchSrvDescriptorHeap = srvHandle;
}

struct RevDescriptorHeapIncrementSizeData* RevEngineFunctions::FindIncrementSizes()
{
	static RevDescriptorHeapIncrementSizeData s_incrementSize = {};
	return &s_incrementSize;
}

void RevEngineFunctions::FindWindowWidthHeight(uint32_t* outX, uint32_t* outY)
{
	REV_ASSERT(outX != nullptr && outY != nullptr);
	*outX = RevEngineMain::s_instance->m_currentWindowWidth;
	*outY = RevEngineMain::s_instance->m_currentWindowHeight;
}

struct ID3D12Device* RevEngineFunctions::FindDevice()
{
	return RevEngineMain::s_instance->m_device;
}

struct ID3D12GraphicsCommandList* RevEngineFunctions::FindCommandList()
{
	return RevEngineMain::s_instance->m_commandList;
}

struct ID3D12CommandQueue* RevEngineFunctions::FindCommandQueue()
{
	return RevEngineMain::s_instance->m_commandQueue;
}

struct ID3D12CommandAllocator* RevEngineFunctions::FindCommandAllocator()
{
	return RevEngineMain::s_instance->m_directCmdListAlloc;
}

struct ID3D12Resource* RevEngineFunctions::FindDepthStencilBuffer()
{
	return RevEngineMain::s_instance->m_depthStencilBuffer;
}

struct ID3D12DescriptorHeap* RevEngineFunctions::FindDSVHeap()
{
	return RevEngineMain::s_instance->m_dsvHeap;
}

class RevShaderManager* RevEngineFunctions::FindRevShaderManager()
{
	return RevEngineMain::s_instance->m_shaderInstance;
}

class RevCamera* RevEngineFunctions::FindCamera()
{
	return RevEngineMain::s_instance->m_camera;
}

struct RevFrameSnapshotData* RevEngineFunctions::FindEngineRuntimeSnapshotData()
{
	return RevEngineMain::s_instance->m_snapshotData;
}

float RevEngineFunctions::GetAspectRatio()
{
	return (float)RevEngineMain::s_instance->m_currentWindowWidth / (float)RevEngineMain::s_instance->m_currentWindowHeight;
}


RevVector2 RevEngineFunctions::GetMouseDelta(bool reset /*= true*/)
{
	RevVector2 returnVector = {};

#if USE_IMGUI
	ImVec2 vector2 = ImGui::GetMouseDragDelta();
	returnVector.m_v[0] = vector2.x;
	returnVector.m_v[1] = vector2.y;
	if (reset)
	{
		ImGui::ResetMouseDragDelta();
	}
#endif
	return returnVector;
}

void RevEngineFunctions::RequestEditorAction(RevEditorActions editorAction)
{
	/*switch (editorAction)
	{
	default:
		DEBUG_ASSERT(0 && "invalid editor action");
		break;
	};*/
}

RevMatrix ComputeBoneMatrix(
	std::vector<RevMatrix>& toParent,
	std::vector<bool>& calcParent,
	std::vector<RevAnimationChannel>& channels,
	std::vector<struct RevBone>& bones,
	float time,
	uint32_t boneIndex,
	UINT firstKeyFrameToUse,
	UINT secondKeyFrameToUse)
{
	RevBone& bone = bones[boneIndex];
	RevMatrix toRoot;
	if (bone.m_parentIndex != UINT8_MAX)
	{
		if (!calcParent[bone.m_parentIndex])
		{
			toParent[boneIndex] = ComputeBoneMatrix(
				toParent,
				calcParent,
				channels,
				bones,
				time,
				bone.m_parentIndex,
				firstKeyFrameToUse,
				secondKeyFrameToUse);

		}

		toRoot = toParent[bone.m_parentIndex];
	}
	else
	{
		toRoot.Identity();
	}


	RevAnimationChannel* channel = bone.m_channelIndex == UINT_MAX ? nullptr : &channels[bone.m_channelIndex];

	RevMatrix boneOffset;
	if (channel)
	{
		const UINT nKeyFramesMinusOne = (UINT)channel->m_keyFrames.size() - 1;
		if (channel->m_keyFrames[0].m_time >= time)
		{
			boneOffset = RevLocationAndQuatenerionToMatrix(channel->m_keyFrames[0].m_rotation, channel->m_keyFrames[0].m_location);
		}
		else
		if (channel->m_keyFrames[nKeyFramesMinusOne].m_time <= time)
		{
			boneOffset = RevLocationAndQuatenerionToMatrix(channel->m_keyFrames[nKeyFramesMinusOne].m_rotation, channel->m_keyFrames[nKeyFramesMinusOne].m_location);
		}
		else
		{
			const RevKeyFrame& keyFrame = channel->m_keyFrames[firstKeyFrameToUse];
			const RevKeyFrame& nextFrame = channel->m_keyFrames[secondKeyFrameToUse];
			float lerpValue = 1 - ((nextFrame.m_time - time) / (nextFrame.m_time - keyFrame.m_time));
			RevQuat quatResult = RevQuatenerionSlerp(keyFrame.m_rotation, nextFrame.m_rotation, lerpValue);
			RevVector3 locationResult = keyFrame.m_location + (nextFrame.m_location - keyFrame.m_location) * lerpValue;
			boneOffset = RevLocationAndQuatenerionToMatrix(quatResult, locationResult);
		}
	}
	else
	{
		boneOffset.Identity();
	}

	toParent[boneIndex] = boneOffset * toRoot;
	calcParent[boneIndex] = true;
	return toParent[boneIndex];
}

void RevEngineFunctions::RequestAnimationUpdate(
		struct RevAnimationUpdateData& animationUpdateData,
		struct RevAnimationInstanceData* animationInstance)
{
	std::vector<RevMatrix> toParent(animationUpdateData.m_bones.size());
	std::vector<bool> calculatedToParent(animationUpdateData.m_bones.size());
	const float time = fmodf(animationUpdateData.m_deltaTime, animationInstance->m_duration);
	UINT indexToUse = (animationInstance->m_channels[0].m_keyFrames.size() - 1) * time / animationInstance->m_duration;
	UINT nextIndex = indexToUse + 1;
	const UINT nBones = (UINT)animationUpdateData.m_bones.size();
	for (UINT boneIndex = 0; boneIndex < nBones; boneIndex++)
	{
		if (!calculatedToParent[boneIndex])
		{
			toParent[boneIndex] = ComputeBoneMatrix(
				toParent,
				calculatedToParent,
				animationInstance->m_channels,
				animationUpdateData.m_bones,
				time,
				boneIndex,
				indexToUse,
				nextIndex);
		}
	}

	for (UINT boneIndex = 0; boneIndex < nBones; boneIndex++)
	{
		animationUpdateData.m_outBoneMatrixes[boneIndex] = animationUpdateData.m_bones[boneIndex].m_offsetMatrix * toParent[boneIndex];
	}
}

void LoadTexturesForModelData(RevTextureInitializationData& initializationData, struct RevModelData* outData)
{
	const UINT numTexturePaths = initializationData.m_nTexturePaths;
	if (numTexturePaths  > 0)
	{
		REV_ASSERT(numTexturePaths == 4);
		RevEngineFunctions::LoadTexture(initializationData.m_diffuseTexturePath, &outData->m_diffuseTexture);
		RevEngineFunctions::LoadTexture(initializationData.m_normalTexturePath, &outData->m_normalTexture );
		RevEngineFunctions::LoadTexture(initializationData.m_substanceTexturePath, &outData->m_substanceTexture);
		RevEngineFunctions::LoadTexture(initializationData.m_roughnessAOEmissivePath, &outData->m_roughnessAoEmissiveTexture);
	}
}

void RevEngineFunctions::CreateNormalModelGeometry(struct RevNormalModelInitializationData& baseData, struct RevModelData* outData)
{
	LoadTexturesForModelData(baseData.m_textureInitialization, outData);
	RevUtils::CreateModelGeometry(
		(void*)&baseData.vertices[0],
		baseData.m_nVertexes,
		sizeof(baseData.vertices[0]),
		(void*)&baseData.indices[0],
		baseData.m_nIndices,
		outData);
}

void RevEngineFunctions::CreateAnimatedModelGeometry(struct RevAnimatedNodelInitializationData& animatedData, struct RevModelData* outData)
{
	LoadTexturesForModelData(animatedData.m_textureInitialization,  outData);
	for (UINT index = 0; index < animatedData.m_nBones; index++)
	{
		outData->m_bones.push_back(animatedData.bones[index]);
	}

	for (UINT index = 0; index < animatedData.instanceData.size(); index++)
	{
		RevAnimationInstanceData* newInstance = new RevAnimationInstanceData();
		*newInstance = animatedData.instanceData[index];
		outData->m_animationInstances.push_back(newInstance);
	}

	RevUtils::CreateModelGeometry(
		(void*)&animatedData.vertices[0],
		animatedData.m_nVertexes,
		sizeof(animatedData.vertices[0]),
		(void*)&animatedData.indices[0],
		animatedData.m_nIndices,
		outData);
}

void RevEngineFunctions::LoadTexture(const char* path, struct ID3D12Resource** resourceToEndUpAt)
{
	std::string texturePath = path;
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, texturePath.c_str(), -1, buffer, 512);
	std::wstring textureFileName = std::wstring(buffer);

	std::vector<D3D12_SUBRESOURCE_DATA> subResources;
	std::unique_ptr<uint8_t[]> ddsData;

	RevThrowIfFailed(LoadDDSTextureFromFile(
		RevEngineFunctions::FindDevice(),
		textureFileName.c_str(),
		resourceToEndUpAt,
		ddsData,
		subResources));
}

ID3D12Resource* RevEngineFunctions::CreateResourceTexture(
	UINT width,
	UINT height,
	UINT64 format,
	UINT64 clearValue)
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = (DXGI_FORMAT)format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = (DXGI_FORMAT)clearValue;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	ID3D12Resource* returnResource = nullptr;

	RevThrowIfFailed(FindDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&optClear,
		IID_PPV_ARGS(&returnResource)));
	return returnResource;
}

RevFrameResource* RevEngineFunctions::FindFrameResource(INT32 frameIndex)
{
	return RevEngineMain::s_instance->m_frameResource[frameIndex];
}

class RevModelManager* RevEngineFunctions::FindModelManager()
{
	return RevEngineMain::s_instance->m_modelManager;
}

RevUIManager* RevEngineFunctions::FindUIManager()
{
	return RevEngineMain::s_instance->m_mainManger->m_uiManager;
}

RevRenderManager* RevEngineFunctions::FindRenderManager()
{
	return RevEngineMain::s_instance->m_renderManager;
}

RevInputManager* RevEngineFunctions::FindInputManager()
{
	return RevEngineMain::s_instance->m_inputManager;
}

RevEngineMode RevEngineFunctions::FindEngineMode()
{
	return RevEngineMain::s_instance->m_engineMode;
}

void RevEngineFunctions::FlushCommandQueue()
{
	RevEngineMain::s_instance->FlushCommandQueue();
}
