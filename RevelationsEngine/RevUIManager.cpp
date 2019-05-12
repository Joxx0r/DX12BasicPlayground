#include "stdafx.h"
#include "RevUIManager.h"
#include "imgui/imgui.h"
#include "RevShadermanager.h"
#include "RevUtils.h"
#include "RevCamera.h"

struct RevModelData* m_modelData = nullptr;
struct ID3D12DescriptorHeap* m_srvHeap = nullptr;
struct ID3D12Resource* m_backBufferResource = nullptr;
ID3D12Resource* m_uploadBuffer = nullptr;

bool GRenderPhysics = false;
bool GRenderFull = true;
bool GRenderWireFrame = false;

// dxgi, device, cmd queue, cmd allocator
// command lists genereation with vsh,psh
#define IMGUI_GPU_BUFFER_SIZE 1024*1024

// For creation of resources in the GPU
struct HeapProperty
{
	enum Enum
	{
		Default,
		Upload,
		ReadBack,

		Count
	};

	D3D12_HEAP_PROPERTIES m_properties;
	D3D12_RESOURCE_STATES m_state;
};

static const HeapProperty s_heapProperties[] =
{
	{ { D3D12_HEAP_TYPE_DEFAULT,  D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 1, 1 }, D3D12_RESOURCE_STATE_COMMON },
	{ { D3D12_HEAP_TYPE_UPLOAD,   D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 1, 1 }, D3D12_RESOURCE_STATE_GENERIC_READ },
	{ { D3D12_HEAP_TYPE_READBACK, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 1, 1 }, D3D12_RESOURCE_STATE_COPY_DEST },
};

static ID3D12Resource* createCommittedResource(ID3D12Device* _device, HeapProperty::Enum _heapProperty, D3D12_RESOURCE_DESC* _resourceDesc, D3D12_CLEAR_VALUE* _clearValue)
{
	const HeapProperty& heapProperty = s_heapProperties[_heapProperty];
	ID3D12Resource* resource;
	RevThrowIfFailed(_device->CreateCommittedResource(&heapProperty.m_properties
		, D3D12_HEAP_FLAG_NONE
		, _resourceDesc
		, heapProperty.m_state
		, _clearValue
		, __uuidof(ID3D12Resource)
		, (void**)&resource
	));

	return resource;
}

static ID3D12Resource* createCommittedResource(ID3D12Device* _device, HeapProperty::Enum _heapProperty, uint64_t _size, D3D12_RESOURCE_FLAGS _flags = D3D12_RESOURCE_FLAG_NONE)
{
	D3D12_RESOURCE_DESC resourceDesc;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = _size;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = _flags;

	return createCommittedResource(_device, _heapProperty, &resourceDesc, NULL);
}

void ImGui_ImplDX12_RenderDrawLists(ImDrawData* _draw_data)
{
	// Range CPU will read from mapping the upload buffer
	// End < Begin specifies CPU will not read the mapped buffer
	D3D12_RANGE readRange;
	readRange.End = 0;
	readRange.Begin = 1;

	char* mappedBuffer = 0;
	m_uploadBuffer->Map(0, &readRange, (void**)&mappedBuffer);
	if (mappedBuffer == NULL)
		return;

	char* writeCursor = mappedBuffer;

	// Copy the projection matrix at the beginning of the buffer
	{
		float translate = -0.5f * 2.f;
		const float L = 0.f;
		const float R = ImGui::GetIO().DisplaySize.x;
		const float B = ImGui::GetIO().DisplaySize.y;
		const float T = 0.f;
		const float mvp[4][4] =
		{
			{ 2.0f / (R - L),   0.0f,           0.0f,       0.0f },
			{ 0.0f,         2.0f / (T - B),     0.0f,       0.0f, },
			{ 0.0f,         0.0f,           0.5f,       0.0f },
			{ (R + L) / (L - R),  (T + B) / (B - T),    0.5f,       1.0f },
		};

		memcpy(writeCursor, &mvp[0], sizeof(mvp));
		writeCursor += sizeof(mvp);
	}

	// Copy the vertices and indices for each command list
	for (int n = 0; n < _draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = _draw_data->CmdLists[n];
		size_t verticesCount = cmd_list->VtxBuffer.size();
		size_t indicesCount = cmd_list->IdxBuffer.size();
		size_t verticesSize = verticesCount * sizeof(ImDrawVert);
		size_t indicesSize = indicesCount * sizeof(ImDrawIdx);

		// Copy the vertex data
		memcpy(writeCursor, &cmd_list->VtxBuffer[0], verticesSize);
		writeCursor += verticesSize;

		// Copy the index data
		memcpy(writeCursor, &cmd_list->IdxBuffer[0], indicesSize);
		writeCursor += indicesSize;
	}

	ID3D12GraphicsCommandList* commandList = RevEngineFunctions::FindCommandList(); 

	D3D12_VIEWPORT viewport;
	viewport.Width = ImGui::GetIO().DisplaySize.x;
	viewport.Height = ImGui::GetIO().DisplaySize.y;
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;
	viewport.TopLeftX = 0.f;
	viewport.TopLeftY = 0.f;

	commandList->RSSetViewports(1, &viewport);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetPipelineState(m_modelData->m_pso);

	D3D12_GPU_VIRTUAL_ADDRESS bufferAddress = m_uploadBuffer->GetGPUVirtualAddress();
	commandList->SetGraphicsRootConstantBufferView(1, bufferAddress);

	uint64_t readCursor = 64; // Our constant buffer takes 64 bytes - one mat4x4


	for (int n = 0; n < _draw_data->CmdListsCount; n++)
	{
		// Render command lists
		int vtx_offset = 0;
		int idx_offset = 0;

		const ImDrawList* cmd_list = _draw_data->CmdLists[n];
		size_t verticesCount = cmd_list->VtxBuffer.size();
		size_t indicesCount = cmd_list->IdxBuffer.size();
		size_t verticesSize = verticesCount * sizeof(ImDrawVert);
		size_t indicesSize = indicesCount * sizeof(ImDrawIdx);

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		vertexBufferView.BufferLocation = bufferAddress + readCursor;
		vertexBufferView.StrideInBytes = sizeof(ImDrawVert);
		vertexBufferView.SizeInBytes = (UINT)verticesSize;
		readCursor += verticesSize;

		D3D12_INDEX_BUFFER_VIEW indexBufferView;
		indexBufferView.BufferLocation = bufferAddress + readCursor;
		indexBufferView.SizeInBytes = (UINT)indicesSize;
		indexBufferView.Format = DXGI_FORMAT_R16_UINT;
		readCursor += indicesSize;

		commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
		commandList->IASetIndexBuffer(&indexBufferView);

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback)
			{
				pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				if (pcmd->TextureId != nullptr)
				{
					D3D12_GPU_DESCRIPTOR_HANDLE handle = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
					handle.ptr += RevEngineFunctions::FindIncrementSizes()->m_cbvSrvUavSize;
					commandList->SetGraphicsRootDescriptorTable(0, handle);
				}
				else
				{
					commandList->SetGraphicsRootDescriptorTable(0,
						m_srvHeap->GetGPUDescriptorHandleForHeapStart());
				}
		
				const D3D12_RECT r = { (LONG)pcmd->ClipRect.x, (LONG)pcmd->ClipRect.y, (LONG)pcmd->ClipRect.z, (LONG)pcmd->ClipRect.w };
				commandList->RSSetScissorRects(1, &r);
				commandList->DrawIndexedInstanced(pcmd->ElemCount, 1, idx_offset, vtx_offset, 0);
			}
			idx_offset += pcmd->ElemCount;
		}
		vtx_offset += (UINT)verticesCount;
	}
}


void RevUIManager::Initialize(void* hwnd)
{
	ID3D12Device* device = RevEngineFunctions::FindDevice();
	ID3D12GraphicsCommandList* commandList = RevEngineFunctions::FindCommandList();
	RevThrowIfFailed(commandList->Reset(RevEngineFunctions::FindCommandAllocator(), nullptr));

	InitializeUIIO(hwnd);
	InitializeRootSignature();
	InitializeShaderInputLayout();

	D3D12_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.f;
	rasterizerDesc.SlopeScaledDepthBias = 0.f;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = true;
	rasterizerDesc.ForcedSampleCount = 1;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	D3D12_BLEND_DESC blendDesc;
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].LogicOpEnable = false;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	RevPSOInitializationData initializationData;
	initializationData.m_shader = m_modelData->m_shader;
	initializationData.m_inputLayoutData = m_modelData->m_inputLayout.data();
	initializationData.m_nInputLayout = (UINT)m_modelData->m_inputLayout.size();
	initializationData.m_pso = &m_modelData->m_pso;
	initializationData.m_blendDesc = &blendDesc;
	initializationData.m_rasterizerDesc = &rasterizerDesc;
	initializationData.m_useDepth = false;
	initializationData.m_useStencil = false;
	initializationData.m_rootSignature = m_modelData->m_rootSignature;
	RevUtils::CreatePSO(initializationData);

	// Create upload ring buffer, which we'll also use as staging buffer for the texture
	m_uploadBuffer = createCommittedResource(device, HeapProperty::Upload, IMGUI_GPU_BUFFER_SIZE * 8);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.f;

	// Create fonts texture and SRV descriptor for it
	unsigned char* pixels = 0;
	int width, height;

	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	// Create fonts texture
	D3D12_RESOURCE_DESC desc;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Alignment = 0;
	desc.Width = width;
	desc.Height = height;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	ID3D12Resource* fontResource = createCommittedResource(device, HeapProperty::Default, &desc, 0);

	// Upload the fonts texture
	uint32_t subres = 0;
	uint32_t numRows;
	uint64_t rowPitch;
	uint64_t totalBytes;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
	device->GetCopyableFootprints(&desc
		, subres
		, 1
		, 0
		, &layout
		, &numRows
		, &rowPitch
		, &totalBytes
	);

	uint8_t* mappedBuffer;

	// Upload the font
	m_uploadBuffer->Map(0, NULL, (void**)&mappedBuffer);
	memcpy(mappedBuffer, pixels, (size_t)totalBytes);
	m_uploadBuffer->Unmap(0, NULL);

	D3D12_BOX box;
	box.left = 0;
	box.top = 0;
	box.right = (UINT)desc.Width;
	box.bottom = (UINT)desc.Height;
	box.front = 0;
	box.back = 1;


	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_TEXTURE_COPY_LOCATION dst = { fontResource,   D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,{ subres } };
	D3D12_TEXTURE_COPY_LOCATION src = { m_uploadBuffer, D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,  layout };
	commandList->CopyTextureRegion(&dst, 0, 0, 0, &src, &box);
	device->CreateShaderResourceView(fontResource, &srvDesc, handle);
	
	handle.ptr += RevEngineFunctions::FindIncrementSizes()->m_cbvSrvUavSize;


	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.PlaneSlice = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.f;

		D3D12_RESOURCE_DESC desc;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Alignment = 0;
		UINT32 width, height;
		RevEngineFunctions::FindWindowWidthHeight(&width, &height);
		desc.Width = width;
		desc.Height = height;

		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		m_backBufferResource = createCommittedResource(device, HeapProperty::Default, &desc, 0);
		device->CreateShaderResourceView(m_backBufferResource, &srvDesc, handle);
	}
	ImGui::StyleColorsClassic();
	RevThrowIfFailed(commandList->Close());
	ID3D12CommandList* cmdsLists[] = {  commandList };
	RevEngineFunctions::FindCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	RevEngineFunctions::FlushCommandQueue();
}

void RevUIManager::InitializeShaderInputLayout()
{
	m_modelData->m_shader = RevShaderManager::GetShader(L"Shaders\\ui.hlsl");
	m_modelData->m_inputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, (size_t)(&((ImDrawVert*)0)->pos), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, (size_t)(&((ImDrawVert*)0)->uv),  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, (size_t)(&((ImDrawVert*)0)->col), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
}

void RevUIManager::InitializeRootSignature()
{
	D3D12_DESCRIPTOR_HEAP_DESC srvDescHeap;
	srvDescHeap.NumDescriptors = 2;
	srvDescHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvDescHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvDescHeap.NodeMask = 0;
	RevThrowIfFailed(RevEngineFunctions::FindDevice()->CreateDescriptorHeap(&srvDescHeap
		, __uuidof(ID3D12DescriptorHeap)
		, (void**)&m_srvHeap
	));

	m_modelData = new RevModelData();

	// A root parameter describes one slot of a root signature
	// Parameter types are: DESCRIPTOR_TABLE, 32BIT_CONSTANTS, CBV, SRV, UAV
	//
	// Root Descriptor Table: { uint NumDescriptorRanges, const DescriptorRange* pDescriptorRanges }
	// Root Descriptor:       { uint ShaderRegister, uint RegisterSpace }
	// Root Constants:        { uint ShaderRegister, uint RegisterSpace, uint Num32BitValues

	D3D12_DESCRIPTOR_RANGE descRange[] =
	{
		{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV,            // Range Type
		1,                                          // Number of descriptors
		0,                                          // Base shader register
		0,                                          // Register space
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND },     // Offset in descriptors from the start of the root signature
	};

	// This root signature will have two parameters, one descriptor table for SRVs and one constant buffer descriptor
	D3D12_ROOT_PARAMETER rootParameter[] =
	{
		{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,{ 1,descRange }, D3D12_SHADER_VISIBILITY_ALL },
		{ D3D12_ROOT_PARAMETER_TYPE_CBV,{ 0,0 }, D3D12_SHADER_VISIBILITY_VERTEX },
	};

	rootParameter[1].Descriptor.RegisterSpace = 0;
	rootParameter[1].Descriptor.ShaderRegister = 0;

	// Include a statci sampler
	D3D12_STATIC_SAMPLER_DESC samplerDesc;
	memset(&samplerDesc, 0, sizeof(samplerDesc));
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.MipLODBias = 0.f;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDesc.MinLOD = 0.f;
	samplerDesc.MaxLOD = 0.f;
	samplerDesc.ShaderRegister = 0;
	samplerDesc.RegisterSpace = 0;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// Root signature description
	D3D12_ROOT_SIGNATURE_DESC descRootSignature;
	descRootSignature.NumParameters = 2;
	descRootSignature.pParameters = rootParameter;
	descRootSignature.NumStaticSamplers = 1;
	descRootSignature.pStaticSamplers = &samplerDesc;
	descRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// Serialise the root signature into memory
	ID3DBlob* outBlob;
	ID3DBlob* errorBlob;
	D3D12SerializeRootSignature(&descRootSignature
		, D3D_ROOT_SIGNATURE_VERSION_1
		, &outBlob
		, &errorBlob
	);

	// Create the root signature using the binary blob
	RevThrowIfFailed(RevEngineFunctions::FindDevice()->CreateRootSignature(0
		, outBlob->GetBufferPointer()
		, outBlob->GetBufferSize()
		, __uuidof(ID3D12RootSignature)
		, (void**)&m_modelData->m_rootSignature
	));

	outBlob->Release();
}

void RevUIManager::InitializeUIIO(void* hwnd)
{
	ImGuiIO& io = ImGui::GetIO();
	io.KeyMap[ImGuiKey_Tab] = VK_TAB;                              // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array that we will update during the application lifetime.
	io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
	io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
	io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
	io.KeyMap[ImGuiKey_Home] = VK_HOME;
	io.KeyMap[ImGuiKey_End] = VK_END;
	io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
	io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
	io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
	io.KeyMap[ImGuiKey_A] = 'A';
	io.KeyMap[ImGuiKey_C] = 'C';
	io.KeyMap[ImGuiKey_V] = 'V';
	io.KeyMap[ImGuiKey_X] = 'X';
	io.KeyMap[ImGuiKey_Y] = 'Y';
	io.KeyMap[ImGuiKey_Z] = 'Z';

	io.RenderDrawListsFn = ImGui_ImplDX12_RenderDrawLists;
	io.ImeWindowHandle = hwnd;
}


void RevUIManager::Draw()
{
	DrawGameWindow();

	ID3D12GraphicsCommandList* commandList = RevEngineFunctions::FindCommandList();
	commandList->SetPipelineState(m_modelData->m_pso);

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_srvHeap };
	commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	commandList->SetGraphicsRootSignature(m_modelData->m_rootSignature);

	commandList->SetGraphicsRootDescriptorTable(0,
		m_srvHeap->GetGPUDescriptorHandleForHeapStart());

	ImGui::Render();
}

void RevUIManager::DrawGameWindow()
{
	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_MenuBar;

	ImGui::SetNextWindowSize(ImVec2(1024, 920), ImGuiCond_Once);
	ImGui::Begin("Game window", nullptr, window_flags);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
	float width = ImGui::GetWindowWidth();
	float height = ImGui::GetWindowHeight();
	float offset = 55;
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("TextureView"))
		{
			extern INT32  GTextureSample;
			if (ImGui::MenuItem("Albedo"))
			{
				GTextureSample = 0;
			}
			if (ImGui::MenuItem("Normal"))
			{
				GTextureSample = 1;
			}

			if (ImGui::MenuItem("Substance"))
			{
				GTextureSample = 2;
			}

			if (ImGui::MenuItem("Roughness"))
			{
				GTextureSample = 3; 
			}

			if (ImGui::MenuItem("AO"))
			{
				GTextureSample = 4;
			}
			if (ImGui::MenuItem("Emissive"))
			{
				GTextureSample = 5;
			}

			if (ImGui::MenuItem("Depth"))
			{
				GTextureSample = 6;
			}
			if (ImGui::MenuItem("ViewSpace"))
			{
				GTextureSample = 7;
			}
			if (ImGui::MenuItem("WorldSpace"))
			{
				GTextureSample = 8;
			}
			if (ImGui::MenuItem("ProjectionSpace"))
			{
				GTextureSample = 9;
			}

			if (ImGui::MenuItem("Lighting"))
			{
				GTextureSample = 10;
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Camera"))
		{
			float baseMovementSpeed = 100;
			extern float GMovementSpeed;
			ImGui::SliderFloat("MovementSpeedModifier", &GMovementSpeed, 10.0f, 100.0f);

			extern float GRotationSpeed;
			ImGui::SliderFloat("RotationSpeed", &GRotationSpeed, 10.0f, 100.0f);

			extern float GAnimationRateScale;
			ImGui::SliderFloat("RateScale", &GAnimationRateScale, 0.0f, 1.f);


			extern RevVector3 GCameraDefaultLocaton;
			ImGui::InputFloat("StartPositionX", &GCameraDefaultLocaton[0]);
			ImGui::InputFloat("StartPositionY", &GCameraDefaultLocaton[1]);
			ImGui::InputFloat("StartPositionZ", &GCameraDefaultLocaton[2]);

			if (ImGui::MenuItem("ResetCameraPosition"))
			{
				RevEngineFunctions::FindCamera()->ResetPosition();
			}
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	ImGui::Image(m_srvHeap, ImVec2(width, height - offset), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(0, 0, 0, 0));
	ImGui::PopStyleVar();
	ImGui::End();

}
void RevUIManager::CopySRV(ID3D12Resource* resource)
{
	ID3D12GraphicsCommandList* commandList = RevEngineFunctions::FindCommandList();
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_backBufferResource,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));

	commandList->CopyResource(m_backBufferResource, resource);

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_backBufferResource,
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

}
