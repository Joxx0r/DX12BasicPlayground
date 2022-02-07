#include "stdafx.h"
#include "RevFrameResource.h"
#include "RevUploadBuffer.h"

RevFrameResource::RevFrameResource(ID3D12Device* device, uint32_t passCount, UINT objectCount, UINT materialCount)
{
	RevThrowIfFailed(device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&CmdListAlloc)));

	m_passCB = new RevUploadBuffer<PassConstants>(device, passCount, true);

	m_materialCB = new RevUploadBuffer<MaterialConstants>(device, materialCount, true);
	m_objectCB = new RevUploadBuffer<ObjectConstants>(device, objectCount, true);
	m_animatedObjectCB = new RevUploadBuffer<ObjectConstantsAnimated>(device, objectCount, true);
}
