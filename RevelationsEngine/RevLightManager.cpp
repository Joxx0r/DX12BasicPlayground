#include "stdafx.h"
#include "RevLightManager.h"
#include "RevUtils.h"
#include "RevShadermanager.h"
#include "RevWorld.h"
#include "RevCamera.h"
#include "RevFrameResource.h"
#include "RevPaths.h"

#define REV_MAX_LIGHT 500

void RevLightManager::Initialize()
{
	m_modelData = new RevModelData();

	std::vector<RevVector3> lightVertex;
	std::vector<UINT> indicies;
	
	UINT index = 0;
		
	const int VertexSize = 12;
	const int PolygonSize = VertexSize - 1;
	float theta = -90.0f * (XM_PI / 180.0f);;
	float phi = 0.0f;
	for (int i = 0; i < VertexSize; i++)
	{
		phi = 0;
		for (int j = 0; j < VertexSize; j++)
		{

			RevVector3 vertex(
				(float)cos(theta) * (float)cos(phi) * 1.03f,
				(float)cos(theta) * (float)sin(phi) * 1.03f,
				(float)sin(theta) * 1.25f);

			lightVertex.push_back(vertex);
			phi += ((360.0f) / (float)PolygonSize) * (XM_PI / 180.0f);
		}
		theta += ((180.0f / (float)PolygonSize)* (XM_PI / 180.0f));
	}

	int indiceCount = 0;
	for (int i = 0; i < PolygonSize; i++)
	{
		for (int j = 0; j < PolygonSize; j++)
		{
			for (UINT d = 0; d < 6; d++)
			{
				indicies.push_back(0);
			}

			indicies[indiceCount + 2] = (UINT)(i * VertexSize + j);
			indicies[indiceCount + 1] = (UINT)(i * VertexSize + j + 1);
			indicies[indiceCount + 0] = (UINT)((i + 1) * VertexSize + j);
			indicies[indiceCount + 5] = (UINT)(i * VertexSize + (j + 1));
			indicies[indiceCount + 4] = (UINT)((i + 1) * VertexSize + (j + 1));
			indicies[indiceCount + 3] = (UINT)((i + 1) * VertexSize + (j));
			indiceCount += 6;
		}
	}

	RevUtils::CreateModelGeometry(
		&lightVertex[0],
		(UINT)lightVertex.size(),
		sizeof(lightVertex[0]),
		&indicies[0],
		(UINT)indicies.size(),
		m_modelData);

	ID3D12Device* device = RevEngineFunctions::FindDevice();

	m_lightCB = new RevUploadBuffer<RevLightConstant>(device, REV_MAX_LIGHT, true);

	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0, 0);
	CD3DX12_DESCRIPTOR_RANGE secondTexTable;
	secondTexTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 0);


	CD3DX12_ROOT_PARAMETER slotRootParameter[4];
	slotRootParameter[0].InitAsDescriptorTable(0, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsDescriptorTable(1, &secondTexTable, D3D12_SHADER_VISIBILITY_PIXEL);

	slotRootParameter[2].InitAsConstantBufferView(0);
	slotRootParameter[3].InitAsConstantBufferView(1);
	RevUtils::CreateModelRootDescription(&slotRootParameter[0], ARRAYSIZE(slotRootParameter), m_modelData);

	HRESULT hr = S_OK;
	m_modelData->m_shader = RevShaderManager::GetShader(RevPaths::AddContentPathWChar(L"Shaders\\light.hlsl").c_str());
	m_modelData->m_inputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "SCALE", 0, DXGI_FORMAT_R32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};


	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;

	RevPSOInitializationData initializationData;
	initializationData.m_shader = m_modelData->m_shader;
	initializationData.m_inputLayoutData = m_modelData->m_inputLayout.data();
	initializationData.m_nInputLayout = (UINT)m_modelData->m_inputLayout.size();
	initializationData.m_rootSignature = m_modelData->m_rootSignature;
	initializationData.m_pso = &m_modelData->m_pso;
	initializationData.m_numRenderTargets = 1;
	initializationData.m_useDepth = true;
	initializationData.m_depthStencilDesc = &depthStencilDesc;
	RevUtils::CreatePSO(initializationData);

}

void RevLightManager::ManageLights(
	class RevCamera* toView,
	class RevWorld* world,
	RevModelFrameRender& renderEntry,
	struct RevDescriptorRuntimeData* m_heapData)
{
	REV_ASSERT(toView);
	REV_ASSERT(world);
	std::vector<struct RevLight*>& lights = world->m_lights;
	const RevMatrix& viewMatrix = toView->m_view;

	for (UINT lightIndex =0; lightIndex < lights.size(); lightIndex++)
	{
		RevLight* light = lights[lightIndex];
		RevLightConstant objConstants;
		objConstants.m.Identity();
		objConstants.m.SetLocation(light->m_location);
		objConstants.m = objConstants.m.Transpose();
		objConstants.m_scale = light->m_radius;
		m_lightCB->CopyData(lightIndex, objConstants);
	}
}
