#include "stdafx.h"
#include "RevRenderManager.h"
#include "RevScene.h"
#include "RevWorld.h"
#include "DirectXColors.h"

void RevRenderManager::Initialize()
{
	ID3D12GraphicsCommandList* commandList = RevEngineFunctions::FindCommandList();
	m_scene = new RevScene();
	m_scene->Initialize();

	m_heapData = new RevDescriptorRuntimeData();

	RevDescriptorInitializationData initializationData = {};
	RevEngineFunctions::FindWindowWidthHeight(&initializationData.m_width, &initializationData.m_height);

	RevDescriptorInitializationData::RevHeapCreation data(true, true, DXGI_FORMAT_R8G8B8A8_UNORM);
	initializationData.m_descrptionCreationData = &data;
	initializationData.m_numDescriptorCreation = 1;
	RevEngineFunctions::CreateSRVRTVDescriptorHeap(initializationData, m_heapData);

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_heapData->m_resource[0],
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE));

	RevThrowIfFailed(commandList->Close());
	ID3D12CommandList* cmdsLists[] = { commandList };
	RevEngineFunctions::FindCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	RevEngineFunctions::FlushCommandQueue();
}

void RevRenderManager::DrawFrame(struct RevModelFrameRender& renderEntry)
{
	ID3D12GraphicsCommandList* commandList = RevEngineFunctions::FindCommandList();

	D3D12_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(renderEntry.m_width);
	viewport.Height = static_cast<float>(renderEntry.m_height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	commandList->RSSetViewports(1, &viewport);

	D3D12_RECT returnRect = { 0, 0, (LONG)renderEntry.m_width, (LONG)renderEntry.m_height };
	commandList->RSSetScissorRects(1, &returnRect);

	m_scene->PreMainPassRender(renderEntry);

	if (renderEntry.m_worldToRender)
	{
		renderEntry.m_worldToRender->Draw(renderEntry);
	}

	m_scene->PostMainPassRender(renderEntry);

	// Indicate a state transition on the resource usage.
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_heapData->m_resource[0],
		D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	// Clear the back buffer and depth buffer.

	float normalClearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	commandList->ClearRenderTargetView(
		CD3DX12_CPU_DESCRIPTOR_HANDLE(m_heapData->m_rtvHeap->GetCPUDescriptorHandleForHeapStart()), normalClearColor, 0, nullptr);
	
	renderEntry.m_commandList->OMSetRenderTargets(1, &CD3DX12_CPU_DESCRIPTOR_HANDLE(m_heapData->m_rtvHeap->GetCPUDescriptorHandleForHeapStart()), true, &RevEngineFunctions::FindDSVHeap()->GetCPUDescriptorHandleForHeapStart());

	m_scene->DrawToMainRTVWithDepth(renderEntry);
	
	m_scene->DrawPostProcess(renderEntry);

	renderEntry.m_commandList->OMSetRenderTargets(1, &CD3DX12_CPU_DESCRIPTOR_HANDLE(m_heapData->m_rtvHeap->GetCPUDescriptorHandleForHeapStart()), true, nullptr);

	m_scene->DrawToMainRTVWithoutDepth(renderEntry);

	renderEntry.m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_heapData->m_resource[0],
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE));
}

void RevRenderManager::CopyFinalResultToBackBuffer(ID3D12Resource* outResource)
{
	ID3D12GraphicsCommandList* commandList = RevEngineFunctions::FindCommandList();
	commandList->CopyResource(outResource, m_heapData->m_resource[0]);
}

