#include "stdafx.h"
#include "RevModel.h"
#include "RevUtils.h"
#include "RevShadermanager.h"
#include "RevFrameResource.h"


bool RevModel::Initialize(RevModelType type)
{
	m_type = type;

	ID3D12Device* device = RevEngineFunctions::FindDevice();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	
	RevPSOInitializationData initializationData = {};
	initializationData.m_shader = m_modelData->m_shader;
	initializationData.m_inputLayoutData = m_modelData->m_inputLayout.data();
	initializationData.m_nInputLayout = (uint32_t)m_modelData->m_inputLayout.size();
	initializationData.m_rootSignature = m_modelData->m_rootSignature;
	initializationData.m_pso= &m_modelData->m_pso;
	initializationData.m_numRenderTargets = 4;
	DXGI_FORMAT formats[] = { 
			DXGI_FORMAT_R8G8B8A8_UNORM, 
			DXGI_FORMAT_R16G16B16A16_FLOAT , 
			DXGI_FORMAT_R8G8B8A8_UNORM , 
			DXGI_FORMAT_R8G8B8A8_UNORM };
	initializationData.m_rtvFormats = &formats[0];
	RevUtils::CreatePSO(initializationData);

	
	ID3D12Resource* resoures[] = { m_modelData->m_diffuseTexture, m_modelData->m_normalTexture, m_modelData->m_substanceTexture, m_modelData->m_roughnessAoEmissiveTexture };
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = ARRAYSIZE(resoures);
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	RevThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_descriptorHeap)));

	//
	// Fill out the heap with actual descriptors.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_descriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (uint32_t i = 0; i < ARRAYSIZE(resoures); i++)
	{
		ID3D12Resource* resource = resoures[i];

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = resource->GetDesc().Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = resource->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		device->CreateShaderResourceView(resource, &srvDesc, hDescriptor);
		hDescriptor.Offset(1, RevEngineFunctions::FindIncrementSizes()->m_cbvSrvUavSize);
	}


	return true;
}

void RevModel::Draw(RevModelFrameRender& renderEntry)
{
	renderEntry.m_commandList->SetPipelineState(m_modelData->m_pso);
	
	RevFrameResource* resource = RevEngineFunctions::FindFrameResource(renderEntry.m_currentRenderFrameResourceIndex);

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_descriptorHeap };
	renderEntry.m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	renderEntry.m_commandList->SetGraphicsRootSignature(m_modelData->m_rootSignature);
	renderEntry.m_commandList->SetGraphicsRootConstantBufferView(
		2, resource->m_passCB->Resource()->GetGPUVirtualAddress());
	
 	renderEntry.m_commandList->IASetVertexBuffers(0, 1, &m_modelData->VertexBufferView());
 	renderEntry.m_commandList->IASetIndexBuffer(&m_modelData->IndexBufferView());
 	renderEntry.m_commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	uint32_t objCBByteSize = m_type == RevModelType::Animated ? RevUtils::CalcConstantBufferByteSize(sizeof(ObjectConstantsAnimated)) : RevUtils::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	auto objectCB = m_type == RevModelType::Animated ? resource->m_animatedObjectCB->Resource() : resource->m_objectCB->Resource();
	// For each render item...
	
	const uint32_t cbvSrvDescriptorSize = RevEngineFunctions::FindIncrementSizes()->m_cbvSrvUavSize;
	const uint32_t endIndex = renderEntry.m_startIndex + renderEntry.m_amountToRender;
	for (	uint32_t objectIndex = renderEntry.m_startIndex; 
			objectIndex < endIndex ; 
			++objectIndex)
	{

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(0, cbvSrvDescriptorSize);

		// Offset to the CBV in the descriptor heap for this object and for this frame resource.
		auto cbvHandle = objectCB->GetGPUVirtualAddress();
		cbvHandle += objectIndex * objCBByteSize;
		renderEntry.m_commandList->SetGraphicsRootDescriptorTable(0, tex);
		renderEntry.m_commandList->SetGraphicsRootConstantBufferView(1, cbvHandle);
		renderEntry.m_commandList->DrawIndexedInstanced(
			m_modelData->m_indexCount,
			1, 0, 0, 0);
	}
}

void RevModel::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0);
	CD3DX12_ROOT_PARAMETER slotRootParameter[3];
	slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsConstantBufferView(0);
	slotRootParameter[2].InitAsConstantBufferView(1);
	RevUtils::CreateModelRootDescription(&slotRootParameter[0], ARRAYSIZE(slotRootParameter), m_modelData);
}

void RevModel::BuildShadersAndInputLayout()
{
	if (m_type == RevModelType::Normal)
	{
		m_modelData->m_shader = RevShaderManager::GetShader(L"Shaders\\color.hlsl");
		m_modelData->m_inputLayout =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
	}
	else
	if(m_type == RevModelType::Animated)
	{
		m_modelData->m_shader = RevShaderManager::GetShader(L"Shaders\\color_animated.hlsl");
		m_modelData->m_inputLayout =
		{
			{ "POSITION",		0,	DXGI_FORMAT_R32G32B32_FLOAT,	0, 0,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEX",			0,	DXGI_FORMAT_R32G32_FLOAT,		0, 12,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL",			0,	DXGI_FORMAT_R32G32B32_FLOAT,	0, 20,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "BINORMAL",		0,	DXGI_FORMAT_R32G32B32_FLOAT,	0, 32,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TANGENT",		0,	DXGI_FORMAT_R32G32B32_FLOAT,	0, 44,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "BONEINFLUENCE",	0,	DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 56,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "BONEAFFECT",		0,	DXGI_FORMAT_R8G8B8A8_UINT,		0, 72,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
	}
	else
	{
		char buffer[512];
		sprintf(buffer, "Non supported model type of %i", (UINT8)m_type);
		REV_ASSERT(0 && "Non supported model type");
	}
}
