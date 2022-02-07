#include "stdafx.h"
#include "RevUtils.h"
#include "d3dcompiler.h"
#include "RevEngineMain.h"
#include "RevShadermanager.h"
#include "d3dx12.h"
#include <array>

ID3DBlob* RevUtils::CompileShader(
	const wchar_t* filename,
	const D3D_SHADER_MACRO* defines,
	const char* entrypoint,
	const char* target)
{


	uint32_t compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = S_OK;

	ID3DBlob* byteCode = nullptr;\
	ID3DBlob* errors;
	hr = D3DCompileFromFile(filename, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entrypoint, target, compileFlags, 0, &byteCode, &errors);

	if (errors != nullptr)
	{/*
		int msgboxID = MessageBox(
			NULL,
			(char*)errors->GetBufferPointer(),
			"Shader error",
			MB_OK
		);*/

		OutputDebugStringA((char*)errors->GetBufferPointer());
	}
		
	RevThrowIfFailed(hr);

	return byteCode;
	
}

ID3DBlob* RevUtils::CompileVertexShader(const wchar_t* fileName)
{
	return CompileShader(fileName, nullptr, "VS", "vs_5_0");
}

ID3DBlob* RevUtils::CompilePixelShader(const wchar_t* fileName)
{
	return CompileShader(fileName, nullptr, "PS", "ps_5_0");
}

struct ID3D12Resource* RevUtils::CreateDefaultBuffer(
		struct ID3D12Device* device, 
		struct ID3D12GraphicsCommandList* cmdList, 
		const void* initData, 
		uint64_t byteSize, 
		struct ID3D12Resource* uploadBuffer)
{

	ID3D12Resource* defaultBuffer;

	// Create the actual default buffer resource.
	RevThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&defaultBuffer)));

	// In order to copy CPU memory data into our default buffer, we need to create
	// an intermediate upload heap. 
	RevThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadBuffer)));


	// Describe the data we want to copy into the default buffer.
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;

	// Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
	// will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
	// the intermediate upload heap data will be copied to mBuffer.
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer,
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources<1>(cmdList, defaultBuffer, uploadBuffer, 0, 0, 1, &subResourceData);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer,
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	// Note: uploadBuffer has to be kept alive after the above function calls because
	// the command list has not been executed yet that performs the actual copy.
	// The caller can Release the uploadBuffer after it knows the copy has been executed.


	return defaultBuffer;
}

void RevUtils::CreateModelGeometry(
		void* vData, 
		uint32_t nVertex, 
		uint32_t vStride, 
		void* iData, 
		UINT nIndex, 
		RevModelData* outData)
{

	ID3D12Device* device = RevEngineFunctions::FindDevice();
	assert(nVertex > 0);
	assert(vStride > 0);

	const UINT vbByteSize = nVertex * vStride;
	const UINT ibByteSize = nIndex * sizeof(UINT);
	RevThrowIfFailed(D3DCreateBlob(vbByteSize, &outData->m_vertexBufferCPUMemory));
	CopyMemory(outData->m_vertexBufferCPUMemory->GetBufferPointer(), vData, vbByteSize);

	RevThrowIfFailed(D3DCreateBlob(ibByteSize, &outData->m_indexBufferCPUMemory));
	CopyMemory(outData->m_indexBufferCPUMemory->GetBufferPointer(), iData, ibByteSize);

	outData->m_vertexBufferGpu = RevUtils::CreateDefaultBuffer(device,
		RevEngineMain::s_instance->m_commandList, vData, vbByteSize, outData->m_vertexBufferUpload);

	outData->m_indexBufferGPU = RevUtils::CreateDefaultBuffer(device,
		RevEngineMain::s_instance->m_commandList, iData, ibByteSize, outData->m_indexBufferUpload);

	outData->m_vertexStride = vStride;
	outData->m_vertexBufferByteSize = vbByteSize;
	outData->m_indexFormat = DXGI_FORMAT_R32_UINT;
	outData->m_indexBufferByteSize = ibByteSize;

	outData->m_indexCount = nIndex;
	outData->m_startIndexLocation = 0;
	outData->m_baseVertexLocation = 0;
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> RevGetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };

}

void RevUtils::CreateModelRootDescription(
		CD3DX12_ROOT_PARAMETER* parameter, 
		UINT nParameters,
		struct RevModelData* outData)
{
	auto staticSamplers = RevGetStaticSamplers();

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
		nParameters,
		parameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ID3DBlob* serializedRootSig = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(
		&rootSigDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&serializedRootSig,
		&errorBlob);

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	RevThrowIfFailed(hr);

	RevThrowIfFailed(RevEngineFunctions::FindDevice()->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&outData->m_rootSignature)));
}

void RevUtils::CreatePSO(
	RevPSOInitializationData& initializationData)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { initializationData.m_inputLayoutData, initializationData.m_nInputLayout };
	psoDesc.pRootSignature = initializationData.m_rootSignature;
	if (initializationData.m_shader->m_vsByteCode)
	{
		psoDesc.VS =
		{
			reinterpret_cast<BYTE*>(initializationData.m_shader->m_vsByteCode->GetBufferPointer()),
			initializationData.m_shader->m_vsByteCode->GetBufferSize()
		};
	}
	if (initializationData.m_shader->m_psByteCode)
	{
		psoDesc.PS =
		{
			reinterpret_cast<BYTE*>(initializationData.m_shader->m_psByteCode->GetBufferPointer()),
			initializationData.m_shader->m_psByteCode->GetBufferSize()
		};
	}
	psoDesc.BlendState = initializationData.m_blendDesc ? CD3DX12_BLEND_DESC(*initializationData.m_blendDesc) : CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState = initializationData.m_rasterizerDesc ? CD3DX12_RASTERIZER_DESC(*initializationData.m_rasterizerDesc) : CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	if (!initializationData.m_useDepth)
	{
		psoDesc.DepthStencilState.DepthEnable = initializationData.m_useDepth;
		psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	}
	else
	{
		psoDesc.DepthStencilState = initializationData.m_depthStencilDesc ? CD3DX12_DEPTH_STENCIL_DESC(*initializationData.m_depthStencilDesc) : CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.DSVFormat = REV_DEPTH_STENCIL_FORMAT;
	}

	psoDesc.DepthStencilState.StencilEnable = initializationData.m_useStencil;

	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = initializationData.m_numRenderTargets;

	for (UINT index = 0; index < initializationData.m_numRenderTargets; index++)
	{
		//todo johlander will not work for deferred once we make stuff look nicer (normal formats). & HDR
		psoDesc.RTVFormats[index] = initializationData.m_rtvFormats ? initializationData.m_rtvFormats[index] : REV_BACK_BUFFER_FORMAT;
	}

	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	RevThrowIfFailed(RevEngineFunctions::FindDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(initializationData.m_pso)));

}
