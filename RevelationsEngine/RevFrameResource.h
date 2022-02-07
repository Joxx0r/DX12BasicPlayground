#pragma once

#include "DirectXMath.h"
#include "DirectXCollision.h"
#include "RevUploadBuffer.h"
#include "RevInstance.h"

using namespace DirectX;


struct PassConstants
{
#if !USE_D3D_MATH
	RevMatrix View;
	RevMatrix InvView;
	RevMatrix Proj;
	RevMatrix InvProj;
	RevMatrix ViewProj;
	RevMatrix InvViewProj;
	RevVector3 EyePosW;
	float cbPerObjectPad1 = 0.0f;
	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float TotalTime = 0.0f;
	float DeltaTime = 0.0f;
	float m_windowWidth = 1024;
	float m_windowHeight = 720;
	INT32 m_debugValue = 0;
	float padb;
	float padz;
	RevVector3 m_frustumCorners1;
	float pad1;
	RevVector3 m_frustumCorners2;
	float pad2;
	RevVector3 m_frustumCorners3;
	float pad3;
	RevVector3 m_frustumCorners4;
#else
	XMFLOAT4X4 View;
	XMFLOAT4X4 InvView;
	XMFLOAT4X4 Proj;
	XMFLOAT4X4 InvProj;
	XMFLOAT4X4 ViewProj;
	XMFLOAT4X4 InvViewProj;
	XMFLOAT3 EyePosW;
	float cbPerObjectPad1 = 0.0f;
	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float TotalTime = 0.0f;
	float DeltaTime = 0.0f;
	float m_windowWidth = 1024;
	float m_windowHeight = 720;
	INT32 m_debugValue = 0;
	float padb;
	float padz;
	XMFLOAT3 m_frustumCorners1;
	float pad1;
	XMFLOAT3 m_frustumCorners2;
	float pad2;
	XMFLOAT3 m_frustumCorners3;
	float pad3;
	XMFLOAT3 m_frustumCorners4;
#endif
};

struct MaterialConstants
{
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = 0.25f;

	// Used in texture mapping.
	XMFLOAT4X4 MatTransform;
};

// Stores the resources needed for the CPU to build the command lists
// for a frame.  
struct RevFrameResource
{
public:

	RevFrameResource(ID3D12Device* device, uint32_t passCount, uint32_t objectCount, uint32_t materialCount);
	RevFrameResource(const RevFrameResource& rhs) = delete;
	RevFrameResource& operator=(const RevFrameResource& rhs) = delete;
	~RevFrameResource() {};

	// We cannot reset the allocator until the GPU is done processing the commands.
	// So each frame needs their own allocator.
	struct ID3D12CommandAllocator* CmdListAlloc;
	RevUploadBuffer<PassConstants>* m_passCB = nullptr;
	RevUploadBuffer<ObjectConstants>* m_objectCB = nullptr;
	RevUploadBuffer<MaterialConstants>* m_materialCB = nullptr;

	RevUploadBuffer<ObjectConstantsAnimated>* m_animatedObjectCB = nullptr;


	// Fence value to mark commands up to this fence point.  This lets us
	// check if these frame resources are still in use by the GPU.
	UINT64 Fence = 0;
};