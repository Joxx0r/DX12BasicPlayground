#pragma once

enum class RevRenderSRVOffsetType
{
	Albedo,
	Normal,
	Substance,
	RoughnessAOEmissive,
	Depth,
	SSAO
	
};

class RevScene
{
public:
	RevScene() {};


	void Initialize();
	void PreMainPassRender(struct RevModelFrameRender& renderEntry);

	void DrawPostProcess(struct RevModelFrameRender& renderEntry);
	void DrawToMainRTVWithoutDepth(struct RevModelFrameRender& renderEntry);
	void DrawToMainRTVWithDepth(struct RevModelFrameRender& renderEntry);
	void PostMainPassRender(struct RevModelFrameRender& renderEntry);

	struct RevModelData* m_modelData = nullptr;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_srvCPU;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_srvGPU;


	struct RevDescriptorRuntimeData* m_heapData = nullptr;

	struct ID3D12PipelineState* m_ssaoPSO = nullptr;
	struct RevShader* m_ssaoShader = nullptr;
	class RevLightManager* m_lightManager = nullptr;

	ID3D12Resource* m_randomTextureResource = nullptr;

};

