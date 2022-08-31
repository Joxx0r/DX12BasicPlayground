#pragma once

enum class RevEditorActions : UINT8;
	
class RevEngineFunctions
{
public:
	static void CreateSRVRTVDescriptorHeap(
			const struct RevDescriptorInitializationData& initializationData, 
			struct RevDescriptorRuntimeData* out);

	static struct RevDescriptorHeapIncrementSizeData* FindIncrementSizes();

	static void FindWindowWidthHeight(uint32_t* outX, uint32_t* outY);

	static struct ID3D12Device* FindDevice();
	static struct ID3D12GraphicsCommandList* FindCommandList();
	static struct ID3D12CommandQueue* FindCommandQueue();
	static struct ID3D12CommandAllocator* FindCommandAllocator();

	static struct ID3D12Resource* FindDepthStencilBuffer();
	static struct ID3D12DescriptorHeap* FindDSVHeap();
	static class RevShaderManager* FindRevShaderManager();
	static class RevCamera* FindCamera();

	static struct RevFrameSnapshotData* FindEngineRuntimeSnapshotData();
	static struct RevFrameResource* FindFrameResource(INT32 frameIndex);
	static class RevModelManager* FindModelManager();
	static class RevUIManager* FindUIManager();
	static class RevInputManager* FindInputManager();
	static void FlushCommandQueue();
	static float GetAspectRatio();
	static RevVector2 GetMouseDelta(bool reset = true);

	static void RequestEditorAction(RevEditorActions editorAction);
	
	static void RequestAnimationUpdate(
		struct RevAnimationUpdateData& animationUpdateData,
		struct RevAnimationInstanceData* animationInstance);

	static void CreateNormalModelGeometry(
		struct RevNormalModelInitializationData& baseData,
		struct RevModelData* outData);

	static void CreateAnimatedModelGeometry(
		struct RevAnimatedNodelInitializationData& animatedData,
		struct RevModelData* outData);

	static void LoadTexture(const char* path, struct ID3D12Resource** resourceToEndUpAt);

	static ID3D12Resource* CreateResourceTexture(
			UINT width, 
			UINT height, 
			UINT64 format, 
			UINT64 clearValue);

};