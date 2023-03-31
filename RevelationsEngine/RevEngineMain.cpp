#include "stdafx.h"
#include "RevEngineMain.h"
#include <DXGI.h>
#include <DirectXColors.h>
#include "RevShadermanager.h"
#include "RevModelManager.h"
#include "RevInstance.h"
#include "RevFrameResource.h"
#include "RevController.h"
#include "RevEngineEditorManager.h"
#include "RevEngineGameManager.h"
#include "RevRenderManager.h"
#include "RevEngineManager.h"
#include "RevInputManager.h"
#include "RevWorldLoader.h"
#include "RevWorld.h"
#include "RevModel.h"
#include "RevPaths.h"
#include "RevPhysicsManager.h"

#define REV_WINDOW_MENU_NAME "RevEngine"
#define REV_WINDOW_CLASS_NAME "RevelationsEngine"
#define REV_WINDOW_TITLE "RevEngine"

#define DRAW_GAME_TO_BUFFER 1

bool GBlockGameInput = false;

RevEngineMain* RevEngineMain::s_instance = nullptr;

using namespace DirectX;

void RevEngineMain::Create(const RevInitializationData& initializationData)
{
	if (s_instance)
	{
		RevEngineMain::Destroy();
	}

	s_instance = new RevEngineMain();
	s_instance->InitializeInternal(initializationData);
}

void RevEngineMain::Destroy()
{
	delete s_instance;
	s_instance = nullptr;
}

RevEngineMain* RevEngineMain::GetInstance()
{
	return s_instance;
}

void RevEngineMain::Update(float deltaTime)
{
	s_instance->UpdateInteral(deltaTime);
}

void RevEngineMain::Draw(float deltaTime)
{
	REV_ASSERT(s_instance);
	s_instance->DrawInternal(deltaTime);
}

void RevEngineMain::Resize(uint32_t width, uint32_t height)
{
	REV_ASSERT(s_instance);
	s_instance->ResizeInteral(width, height);
}

float RevEngineMain::GetAspectRatio()
{
	return (float)s_instance->m_currentWindowWidth / (float)s_instance->m_currentWindowHeight;
}

XMFLOAT2 RevEngineMain::GetRenderTargetSize()
{
	XMFLOAT2 returnValue = XMFLOAT2((float)s_instance->m_currentWindowWidth, (float)s_instance->m_currentWindowHeight);
	return returnValue;
}

LRESULT RevEngineMain::ManageWindowsMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return RevEngineManager::ManageWindowsMessage(hwnd, msg, wParam, lParam);
}

void RevEngineMain::LoadWorld(const char* filePath)
{
	s_instance->LoadWorldInternal(filePath);
}

void RevEngineMain::InitializeInternal(const RevInitializationData& initializationData)
{
	m_currentWindowWidth = initializationData.m_windowWidth;
	m_currentWindowHeight = initializationData.m_windowHeight;
	m_engineMode = initializationData.m_engineMode;

	CreateEngineWindow(initializationData);
	CreateD3D(initializationData);
	ResizeInteral(initializationData.m_windowWidth, initializationData.m_windowHeight);

	m_shaderInstance = new RevShaderManager();
	m_shaderInstance->Initialize();

	m_modelManager = new RevModelManager();
	m_modelManager->Initialize();

	m_controller = new RevController();
	m_controller->Initialize();
	
	m_physicsManager = new RevPhysicsManager();
	m_physicsManager->Initialize();

	for (UINT resourceIndex = 0; resourceIndex < REV_FRAME_RESOURCE_COUNT; resourceIndex++)
	{
		m_frameResource.push_back(new RevFrameResource(m_device, 1, AMOUNT_OF_MAX_INSTANCES, 1));
	}

	UINT resources = (AMOUNT_OF_MAX_MODELS + 1) * REV_FRAME_RESOURCE_COUNT;
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = resources;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	RevThrowIfFailed(m_device->CreateDescriptorHeap(&cbvHeapDesc,
		IID_PPV_ARGS(&m_cbvHeap)));

	m_snapshotData = new RevFrameSnapshotData();
	m_renderManager = new RevRenderManager();
	m_renderManager->Initialize();
	if(m_engineMode == RevEngineMode::Editor)
	{
		m_mainManger = new RevEngineEditorManager();
	}
	else
	{
		m_mainManger = new RevEngineGameManager();
	}
	m_mainManger->Initialize();

	m_worldLoader = new RevWorldLoader();
	m_worldLoader->Initialize();

	m_inputManager = new RevInputManager();

	if(m_engineMode == RevEngineMode::Editor)
	{
		LoadWorldInternal(RevPaths::AddLevelPath("engine_editor_default").c_str());
	}
	else
	{
		LoadWorldInternal(RevPaths::AddLevelPath("engine_editor_default").c_str());
	}

	extern bool GDrawGameWindow;
	GDrawGameWindow = m_engineMode == RevEngineMode::Editor;
	//todo johlander cleanup this code (for now only one instance is active so we use that one)
	if(m_activeWorld->m_instances.size() > 0)
	{
		m_controller->SetInstance(m_activeWorld->m_instances[0]);
	}
}

void RevEngineMain::CreateEngineWindow(const RevInitializationData& initializationData)
{
	WNDCLASS wcex = {};

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = initializationData.m_windowHandle;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = initializationData.m_instance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = REV_WINDOW_MENU_NAME;// MAKEINTRESOURCEW(IDI_APPLICATION);
	wcex.lpszClassName = REV_WINDOW_CLASS_NAME;
	
	RegisterClass(&wcex);

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, (LONG)initializationData.m_windowWidth, (LONG)initializationData.m_windowHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	m_windowHandle = CreateWindow(REV_WINDOW_CLASS_NAME, REV_WINDOW_TITLE, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, initializationData.m_instance, nullptr);

	if (!m_windowHandle)
	{
		REV_ASSERT(0 && "Failed creating window");
		return;
	}

	ShowWindow(m_windowHandle, 1);
	UpdateWindow(m_windowHandle);
}

void RevEngineMain::CreateD3D(const RevInitializationData& initializationData)
{
#if defined(DEBUG) || defined(_DEBUG) 
	// Enable the D3D12 debug layer.
	{
		ID3D12Debug* debugController;

		D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
		debugController->EnableDebugLayer();
	}
#endif

	CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory));

	// Try to create hardware device.
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,             // default adapter
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&m_device));

	RevThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&m_fence)));

	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_cbvSrvUavDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = REV_BACK_BUFFER_FORMAT;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	RevThrowIfFailed(m_device->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)));


	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	RevThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

	RevThrowIfFailed(m_device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&m_directCmdListAlloc)));

	RevThrowIfFailed(m_device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_directCmdListAlloc, // Associated command allocator
		nullptr,                   // Initial PipelineStateObject
		IID_PPV_ARGS(&m_commandList)));

	if (m_commandList)
	{
		m_commandList->Close();
	}
	
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = initializationData.m_windowWidth;
	sd.BufferDesc.Height = initializationData.m_windowHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = REV_BACK_BUFFER_FORMAT;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = REV_SWAP_CHAIN_COUNT;
	sd.OutputWindow = m_windowHandle;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Note: Swap chain uses queue to perform flush.
	RevThrowIfFailed(m_dxgiFactory->CreateSwapChain(
		m_commandQueue,
		&sd,
		&m_swapChain));

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = REV_SWAP_CHAIN_COUNT + 1;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	RevThrowIfFailed(m_device->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	RevThrowIfFailed(m_device->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));


	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvHeapDesc.NodeMask = 0;
	RevThrowIfFailed(RevEngineFunctions::FindDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));

	RevDescriptorHeapIncrementSizeData* incrementSizeData = RevEngineFunctions::FindIncrementSizes();
	incrementSizeData->m_cbvSrvUavSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	incrementSizeData->m_rtvSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	incrementSizeData->m_dsvSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void RevEngineMain::FlushCommandQueue()
{
	m_currentFence++;
	RevThrowIfFailed(m_commandQueue->Signal(m_fence, m_currentFence));
	if (m_fence->GetCompletedValue() < m_currentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		RevThrowIfFailed(m_fence->SetEventOnCompletion(m_currentFence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}



void RevEngineMain::DrawInternal(float deltaTime)
{
	RevFrameResource* frameResource = m_frameResource[m_currentFrameResourceIndex];
	// Wait until the GPU has completed commands up to this fence point.
	if (m_fence->GetCompletedValue() < frameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current fence.  
		RevThrowIfFailed(m_fence->SetEventOnCompletion(frameResource->Fence, eventHandle));

		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	RevThrowIfFailed(frameResource->CmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	RevThrowIfFailed(m_commandList->Reset(frameResource->CmdListAlloc, nullptr));

	// Indicate a state transition on the resource usage.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST));
	// Clear the back buffer and depth buffer.
	m_commandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	m_controller->Update(deltaTime);
	if (m_activeWorld)
	{
		m_activeWorld->UpdateRendererData(frameResource, deltaTime);
	}

	RevModelFrameRender render = {};
	render.m_amountToRender = 1;
	render.m_commandList = m_commandList;
	render.m_currentRenderFrameResourceIndex = m_currentFrameResourceIndex;
	render.m_worldToRender = m_activeWorld;
	render.m_width = m_currentWindowWidth;
	render.m_height = m_currentWindowHeight;

	m_controller->Draw(deltaTime, render);
	m_renderManager->DrawFrame(render);

	extern bool GDrawGameWindow;
	if(!GDrawGameWindow)
	{
		m_renderManager->CopyFinalResultToBackBuffer(CurrentBackBuffer());

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET));
	}
	else
	{
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET));

		float normalClearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		m_commandList->ClearRenderTargetView(CurrentBackBufferView(), normalClearColor, 0, nullptr);
	}

	// Specify the buffers we are going to render to.
	m_commandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	m_mainManger->Draw(deltaTime);

	// Indicate a state transition on the resource usage.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Done recording commands.
	RevThrowIfFailed(m_commandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { m_commandList };
	m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// swap the back and front buffers
	RevThrowIfFailed(m_swapChain->Present(0, 0));
	m_currentBackBuffer = (m_currentBackBuffer + 1) % REV_SWAP_CHAIN_COUNT;

	m_currentFence++;
	m_currentFrameResourceIndex = (m_currentFrameResourceIndex + 1 ) % m_frameResource.size();
	frameResource->Fence = m_currentFence;

	// Add an instruction to the command queue to set a new fence point.  Because we 
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	RevThrowIfFailed(m_commandQueue->Signal(m_fence, m_currentFence));
}

void RevEngineMain::UpdateInteral(float deltaTime)
{
	UpdateFrameSnapshotData(deltaTime);

	m_mainManger->Update(deltaTime, m_windowHandle);
}

void RevEngineMain::UpdateFrameSnapshotData(float deltaTime)
{
	m_snapshotData->m_deltaTime = deltaTime;
	static float accumFrame = 0.0f;
	accumFrame += deltaTime;
	//framerate
	{
		if (accumFrame >= 1.0f || m_snapshotData->m_fps == -1.0f)
		{
			m_snapshotData->m_fps = 1 / deltaTime;
			accumFrame = 0.0f;
		}
	}

}

void RevEngineMain::LoadWorldInternal(const char* path)
{
	if (m_activeWorld)
	{
		m_activeWorld->DestoyWorld();
		m_activeWorld = nullptr;
	}

	REV_ASSERT(m_activeWorld == nullptr);
	REV_ASSERT(m_worldLoader);
	m_activeWorld = m_worldLoader->LoadWorld(path);
}

struct ID3D12Resource* RevEngineMain::CurrentBackBuffer() const
{
	return m_swapChainBuffer[m_currentBackBuffer];
}

D3D12_CPU_DESCRIPTOR_HANDLE RevEngineMain::CurrentBackBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_currentBackBuffer,
		m_rtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE RevEngineMain::DepthStencilView() const
{
	return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void RevEngineMain::ResizeInteral(UINT windowWidth, UINT windowHeight)
{
	m_currentWindowWidth = windowWidth;
	m_currentWindowHeight = windowHeight;

	REV_ASSERT(m_device);
	REV_ASSERT(m_swapChain);
	REV_ASSERT(m_directCmdListAlloc);

	// Flush before changing any resources.
	FlushCommandQueue();

	RevThrowIfFailed(
		m_commandList->Reset(m_directCmdListAlloc, nullptr));

	// Release the previous resources we will be recreating.
	for (int i = 0; i < REV_SWAP_CHAIN_COUNT; ++i)
	{
		if (ID3D12Resource* resource = m_swapChainBuffer[i])
		{
			resource->Release();
			m_swapChainBuffer[i] = nullptr;
		}
	}
	if (m_depthStencilBuffer)
	{
		m_depthStencilBuffer->Release();
		m_depthStencilBuffer = nullptr;
	}

	// Resize the swap chain.
	RevThrowIfFailed(m_swapChain->ResizeBuffers(
		REV_SWAP_CHAIN_COUNT,
		m_currentWindowWidth, m_currentWindowHeight,
		REV_BACK_BUFFER_FORMAT,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	m_currentBackBuffer = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < REV_SWAP_CHAIN_COUNT; i++)
	{
		RevThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_swapChainBuffer[i])));
		m_device->CreateRenderTargetView(m_swapChainBuffer[i], nullptr, rtvHeapHandle);
		rtvHeapHandle.ptr += m_rtvDescriptorSize;
	}

	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = m_currentWindowWidth;
	depthStencilDesc.Height = m_currentWindowHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;

	// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
	// the depth buffer.  Therefore, because we need to create two views to the same resource:
	//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
	// we need to create the depth buffer resource with a typeless format.  
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = REV_DEPTH_STENCIL_FORMAT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	RevThrowIfFailed(m_device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(&m_depthStencilBuffer)));

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = REV_DEPTH_STENCIL_FORMAT;
	dsvDesc.Texture2D.MipSlice = 0;
	m_device->CreateDepthStencilView(m_depthStencilBuffer, &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_RESOURCE_BARRIER resourcebarrier = {};
	resourcebarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	resourcebarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resourcebarrier.Transition.pResource = m_depthStencilBuffer;
	resourcebarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	resourcebarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	resourcebarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// Transition the resource from its initial state to be used as a depth buffer.
	m_commandList->ResourceBarrier(1, &resourcebarrier);

	// Execute the resize commands.
	RevThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_commandList };
	m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);	

}

RevEngineMain::RevEngineMain()
{
	memset(this, 0, sizeof(*this));
}

