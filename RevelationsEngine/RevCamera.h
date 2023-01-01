#pragma once

#include "DirectXMath.h"
#include "RevUploadBuffer.h"
#include "MathLib/RevMatrix.h"

using namespace DirectX;

class RevCamera
{
public:
	void Initialize(const RevVector& startLocation);

	void UpdateLocation(float deltaTime);
	void UpdateRendererData(class RevUploadBuffer<struct PassConstants>* passConstant);
	void ResetPosition();

	void UpdateD3DView();


#if !USE_D3D_MATH
	RevMatrix m_view;
	RevMatrix m_proj;
#else
#if !USE_MODIFIED_MATH
	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_proj;
#else
	RevMatrix m_view;
	RevMatrix m_proj;
#endif
#endif

private:

	void UpdateViewInternal();

	RevVector m_startLocation = {};

	float m_x = 0.0f;
	float m_y = 15.0f;
	float m_z = -152;

	float m_pitch = 0.0f;
	float m_yaw = 0.0f;
	float m_roll = 0.0f;
	




};
