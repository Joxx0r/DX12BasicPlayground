#pragma once

#include "RevUploadBuffer.h"
#include "MathLib/RevMatrix.h"

struct ObjectConstants
{
	RevMatrix WorldViewProj;
};

struct ObjectConstantsAnimated
{
	RevMatrix WorldViewProj;
	RevMatrix m_bones[REV_MAX_BONES_MODEL];
};

class RevInstance
{
public:
	RevInstance();

	void Initialize(const class RevMatrix& transform, UINT index, const char* modelPath);
	void Update(struct RevFrameResource* resource, float deltaTime);
	void UpdateLocationInput(float deltaTime, const RevInputData& inputData);
	void Draw(struct RevModelFrameRender& param);

	uint32_t m_modelHandle = UINT32_MAX;
	class RevUploadBuffer<ObjectConstants>* m_objectConstantBuffer = nullptr;

	RevMatrix m_transform;
	uint32_t m_cbufferIndex = 0;

	float m_deltaTime = 0.0f;
};