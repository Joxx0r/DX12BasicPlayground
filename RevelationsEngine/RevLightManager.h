#pragma once

#include "RevUploadBuffer.h"
#include "Mathlib/RevMatrix.h"

struct RevLightConstant
{
	RevMatrix m;
	float m_scale;
};

class RevLightManager
{
public:

	void Initialize();

	void ManageLights(
		class RevCamera* toView,
		class RevWorld* world,
		RevModelFrameRender& renderEntry,
		struct RevDescriptorRuntimeData* m_heapData
	);


protected:

	struct RevModelData* m_modelData = nullptr;
	struct RevDescriptorRuntimeData* m_heapData = nullptr;

	RevUploadBuffer<RevLightConstant>* m_lightCB = nullptr;

};