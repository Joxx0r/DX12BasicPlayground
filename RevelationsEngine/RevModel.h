#pragma once

#define AMOUNT_OF_MAX_MODELS 100

enum class RevModelType : UINT8;

class RevModel
{
public:
	virtual bool Initialize(RevModelType type);
	virtual void Draw(struct RevModelFrameRender& renderEntry);

	void BuildRootSignature();
	void BuildShadersAndInputLayout();

	struct RevModelData* m_modelData = nullptr;

	ID3D12DescriptorHeap* m_descriptorHeap = nullptr;
	RevModelType m_type;
};