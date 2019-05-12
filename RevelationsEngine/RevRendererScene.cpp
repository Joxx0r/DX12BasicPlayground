#include "stdafx.h"
#include "RevRendererScene.h"

#include "d3dcompiler.h"
#include "RevShadermanager.h"
#include "RevFrameResource.h"
#include "DirectXMath.h"
#include <array>
#include "RevUIManager.h"
#include "RevLightManager.h"

void RevRendererScene::Initialize()
{
	ID3D12GraphicsCommandList* commandList = RevEngineFunctions::FindCommandList();
	RevThrowIfFailed(commandList->Reset(RevEngineFunctions::FindCommandAllocator(), nullptr));

	m_modelData = new RevModelData();
	m_heapData = new RevDescriptorRuntimeData();
	
	{
		RevDescriptorInitializationData initializationData = {};
		RevEngineFunctions::FindWindowWidthHeight(&initializationData.m_width, &initializationData.m_height);
		RevDescriptorInitializationData::RevHeapCreation data[] =
		{
			{ true, true, DXGI_FORMAT_R8G8B8A8_UNORM, nullptr },
			{ true, true, DXGI_FORMAT_R16G16B16A16_FLOAT, nullptr },
			{ true, true, DXGI_FORMAT_R8G8B8A8_UNORM, nullptr },
			{ true, true, DXGI_FORMAT_R8G8B8A8_UNORM, nullptr }
		};
		initializationData.m_descrptionCreationData = &data[0];
		initializationData.m_numDescriptorCreation = ARRAYSIZE(data);
		initializationData.m_additionalSRVSize = 2;
		initializationData.m_additionalRTVSize = 1;
		RevEngineFunctions::CreateSRVRTVDescriptorHeap(initializationData, m_heapData);
	}

	//root signature
	{
		CD3DX12_DESCRIPTOR_RANGE texTable;
		texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0, 0);

		CD3DX12_DESCRIPTOR_RANGE secondTexTable;
		secondTexTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 0);

		CD3DX12_DESCRIPTOR_RANGE thirdTexTAble;
		thirdTexTAble.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5, 0);


		CD3DX12_ROOT_PARAMETER slotRootParameter[4];
		slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[1].InitAsDescriptorTable(1, &secondTexTable, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[2].InitAsConstantBufferView(1);
		slotRootParameter[3].InitAsDescriptorTable(1, &thirdTexTAble, D3D12_SHADER_VISIBILITY_PIXEL);
		RevUtils::CreateModelRootDescription(&slotRootParameter[0], ARRAYSIZE(slotRootParameter), m_modelData);
	}


	//geometry
	{
		float vertedPosition = 1.0f;
		std::array<XMFLOAT2, 4> vertices =
		{
			XMFLOAT2(-vertedPosition, -vertedPosition),
			XMFLOAT2(-vertedPosition, vertedPosition),
			XMFLOAT2(vertedPosition, vertedPosition) ,
			XMFLOAT2(vertedPosition, -vertedPosition)
		};

		UINT indices[] =
		{
			// front face
			0, 1, 2,
			0, 2, 3,
		};

		RevUtils::CreateModelGeometry(
			&vertices[0],
			(UINT)vertices.size(),
			sizeof(vertices[0]),
			&indices[0],
			ARRAYSIZE(indices),
			m_modelData);
	}

	RevPSOInitializationData initializationData = {};
	//pso for fullscreen pass
	{
		m_modelData->m_shader = RevShaderManager::GetShader(L"Shaders\\FullscreenTest.hlsl");
		m_modelData->m_inputLayout =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		initializationData.m_shader = m_modelData->m_shader;
		initializationData.m_inputLayoutData = m_modelData->m_inputLayout.data();
		initializationData.m_nInputLayout = (UINT)m_modelData->m_inputLayout.size();
		initializationData.m_rootSignature = m_modelData->m_rootSignature;
		initializationData.m_pso = &m_modelData->m_pso;
		initializationData.m_numRenderTargets = 1;
		initializationData.m_useDepth = false;
		initializationData.m_useStencil = false;
		RevUtils::CreatePSO(initializationData);
	}


	m_lightManager = new RevLightManager();
	m_lightManager->Initialize();
}

void RevRendererScene::PreMainPassRender(struct RevModelFrameRender& renderEntry)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_heapData->m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

	// Indicate a state transition on the resource usage.
	renderEntry.m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_heapData->m_resource[0],
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	renderEntry.m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_heapData->m_resource[1],
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	renderEntry.m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_heapData->m_resource[2],
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	renderEntry.m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_heapData->m_resource[3],
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	/*
		renderEntry.m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_randomTextureResource,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	*/
	// Clear the back buffer and depth buffer.
	float normalClearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	renderEntry.m_commandList->ClearRenderTargetView(handle, normalClearColor, 0, nullptr);

	handle.ptr += RevEngineFunctions::FindIncrementSizes()->m_cbvSrvUavSize;
	renderEntry.m_commandList->ClearRenderTargetView(handle, normalClearColor, 0, nullptr);

	handle.ptr += RevEngineFunctions::FindIncrementSizes()->m_cbvSrvUavSize;
	renderEntry.m_commandList->ClearRenderTargetView(handle, normalClearColor, 0, nullptr);

	handle.ptr += RevEngineFunctions::FindIncrementSizes()->m_cbvSrvUavSize;
	renderEntry.m_commandList->ClearRenderTargetView(handle, normalClearColor, 0, nullptr);

	// Specify the buffers we are going to render to.
	renderEntry.m_commandList->OMSetRenderTargets(4, &m_heapData->m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), true, &RevEngineFunctions::FindDSVHeap()->GetCPUDescriptorHandleForHeapStart());
}

void RevRendererScene::DrawPostProcess(struct RevModelFrameRender& renderEntry)
{
/*	renderEntry.m_commandList->SetPipelineState(m_ssaoPSO);

	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_heapData->m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += RevEngineFunctions::FindIncrementSizes()->m_cbvSrvUavSize * (UINT)RevRenderSRVOffsetType::SSAO;


	renderEntry.m_commandList->OMSetRenderTargets(1, &handle, true, nullptr);

	renderEntry.m_commandList->DrawIndexedInstanced(
		m_modelData->m_indexCount,
		1, 0, 0, 0);*/
}

void RevRendererScene::DrawToMainRTVWithoutDepth(struct RevModelFrameRender& renderEntry)
{
	renderEntry.m_commandList->DrawIndexedInstanced(
		m_modelData->m_indexCount,
		1, 0, 0, 0);
}

void RevRendererScene::DrawToMainRTVWithDepth(struct RevModelFrameRender& renderEntry)
{
	m_lightManager->ManageLights(
		RevEngineFunctions::FindCamera(),
		renderEntry.m_worldToRender,
		renderEntry,
		m_heapData);
}

void RevRendererScene::PostMainPassRender(struct RevModelFrameRender& renderEntry)
{
	renderEntry.m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_heapData->m_resource[0],
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	renderEntry.m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_heapData->m_resource[1],
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	renderEntry.m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_heapData->m_resource[2],
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	renderEntry.m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_heapData->m_resource[3],
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_heapData->m_srvDescriptorHeap };
	renderEntry.m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	renderEntry.m_commandList->SetPipelineState(m_modelData->m_pso);
	renderEntry.m_commandList->SetGraphicsRootSignature(m_modelData->m_rootSignature);
	renderEntry.m_commandList->SetGraphicsRootDescriptorTable(0,
		m_heapData->m_srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	D3D12_GPU_DESCRIPTOR_HANDLE descriptorHeap = m_heapData->m_srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	
	descriptorHeap.ptr += RevEngineFunctions::FindIncrementSizes()->m_cbvSrvUavSize * (UINT)RevRenderSRVOffsetType::Depth;
	renderEntry.m_commandList->SetGraphicsRootDescriptorTable(1,
		descriptorHeap);

	renderEntry.m_commandList->SetGraphicsRootConstantBufferView(
		2,
		RevEngineFunctions::FindFrameResource(renderEntry.m_currentRenderFrameResourceIndex)->m_passCB->Resource()->GetGPUVirtualAddress());

	renderEntry.m_commandList->IASetVertexBuffers(0, 1, &m_modelData->VertexBufferView());
	renderEntry.m_commandList->IASetIndexBuffer(&m_modelData->IndexBufferView());
	renderEntry.m_commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
