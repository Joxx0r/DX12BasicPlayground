#pragma once
#include "stdafx.h"
#include "RevCamera.h"
#include "RevFrameResource.h"
#include "RevTypes.h"
#include "MathLib/RevMath.h"

using namespace DirectX;

float GMovementSpeed = 50;

float GRotationSpeed = 50;
INT32 GTextureSample = 10;

const float FOV = 45;
const float farz = 1000.0f;

RevVector3 GCameraDefaultLocaton(0, 1, 4);

void RevCamera::Initialize()
{
	m_x = GCameraDefaultLocaton.X();
	m_y = GCameraDefaultLocaton.Y();
	m_z = GCameraDefaultLocaton.Z();

#if !USE_D3D_MATH
	XMMATRIX proj = XMMatrixPerspectiveFovLH(FOV * REV_ANGLE_TO_RADIAN , RevEngineFunctions::GetAspectRatio(), 1.0f, farz);

	XMFLOAT4X4 d3d_m_view;
	XMStoreFloat4x4(&d3d_m_view, proj);
	m_proj.Load(&d3d_m_view.m[0][0]);


	RevVector3 pos = RevVector3(m_x, m_y, m_z);
	RevVector3 towards = (RevVector3(m_x, m_y, m_z - 1) - pos);
	towards.Normalize();
	m_view = RevCreateLookAt(towards);
	m_view.SetLocation(pos);
#else
#if USE_MODIFIED_MATH
	XMMATRIX proj = XMMatrixPerspectiveFovLH(FOV * REV_ANGLE_TO_RADIAN, RevEngineFunctions::GetAspectRatio(), 1.0f, farz);

	XMFLOAT4X4 d3d_m_view;
	XMStoreFloat4x4(&d3d_m_view, proj);
	m_proj.Load(&d3d_m_view.m[0][0]);


	RevVector3 pos = RevVector(m_x, m_y, m_z, 1.0f);
	RevVector3 towards = (RevVector(m_x, m_y, m_z - 1) - pos);
	towards.Normalize();
	m_view = RevCreateLookAt(towards);
	m_view.SetLocation(pos);
#else

	XMMATRIX proj = XMMatrixPerspectiveFovLH(FOV * REV_ANGLE_TO_RADIAN, RevEngineFunctions::GetAspectRatio, 1.0f, farz);
	XMStoreFloat4x4(&m_proj, proj);

	XMVECTOR pos = XMVectorSet(m_x, m_y, m_z, 1.0f);
	XMVECTOR towards = XMVectorSet(m_x, m_y, m_z + 1, 1.0f);

	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMMATRIX viewMatrix = XMMatrixLookAtLH(pos, towards, up);
	XMStoreFloat4x4(&m_view, viewMatrix);
#endif
#endif
}


void RevCamera::UpdateLocation(float deltaTime)
{
#if !USE_D3D_MATH || USE_MODIFIED_MATH
	RevVector3 forward = m_view.GetAxis(RevMatrixAxis::Forward);
	RevVector3 right = m_view.GetAxis(RevMatrixAxis::Right);
	RevVector3 up = m_view.GetAxis(RevMatrixAxis::Up);

	RevVector3 currentLocation = m_view.GetLocation();
	extern bool GBlockGameInput;
	if (!GBlockGameInput)
	{
		float movementSpeed = GMovementSpeed;

		float rotationSpeed = GRotationSpeed;
		if ((GetKeyState(REV_VK_A) >> 15) & 1)
		{
			currentLocation += -right * movementSpeed * deltaTime;
		}

		if ((GetKeyState(REV_VK_D) >> 15) & 1)
		{
			currentLocation += right * movementSpeed* deltaTime;
		}

		if ((GetKeyState(REV_VK_Q) >> 15) & 1)
		{
			currentLocation += -up * movementSpeed* deltaTime;
		}

		if ((GetKeyState(REV_VK_E) >> 15) & 1)
		{
			currentLocation += up* movementSpeed* deltaTime;
		}

		if ((GetKeyState(REV_VK_W) >> 15) & 1)
		{
			currentLocation += forward* movementSpeed* deltaTime;
		}

		if ((GetKeyState(REV_VK_S) >> 15) & 1)
		{
			currentLocation += -forward* movementSpeed* deltaTime;
		}

		if ((GetKeyState(VK_UP) >> 15) & 1)
		{
			m_view = RevCreateRotationFromX(-rotationSpeed * deltaTime) * m_view;
		}
		if ((GetKeyState(VK_DOWN) >> 15) & 1)
		{
			m_view = RevCreateRotationFromX(rotationSpeed * deltaTime) * m_view;
		}
		if ((GetKeyState(VK_RIGHT) >> 15) & 1)
		{
			m_view = RevCreateRotationFromY(rotationSpeed * deltaTime) * m_view;
		}
		if ((GetKeyState(VK_LEFT) >> 15) & 1)
		{
			m_view = RevCreateRotationFromY(-rotationSpeed * deltaTime) * m_view;
		}
	}
	
	m_view.SetLocation(currentLocation);

	RevFrameSnapshotData* data = RevEngineFunctions::FindEngineRuntimeSnapshotData();
	data->m_cameraLocation = currentLocation;

#else
	extern bool GBlockGameInput;
	if (!GBlockGameInput)
	{
		float movementSpeed = GMovementSpeed;

		float rotationSpeed = GRotationSpeed;
		if ((GetKeyState(REV_VK_A) >> 15) & 1)
		{
			m_x +=  movementSpeed * deltaTime;
		}

		if ((GetKeyState(REV_VK_D) >> 15) & 1)
		{
			m_x -= movementSpeed * deltaTime;
		}

		if ((GetKeyState(REV_VK_Q) >> 15) & 1)
		{
			m_y += movementSpeed * deltaTime;
		}

		if ((GetKeyState(REV_VK_E) >> 15) & 1)
		{
			m_y -= movementSpeed * deltaTime;
		}

		if ((GetKeyState(REV_VK_W) >> 15) & 1)
		{
			m_z += movementSpeed * deltaTime;
		}

		if ((GetKeyState(REV_VK_S) >> 15) & 1)
		{
			m_z -= movementSpeed * deltaTime;
		}
	}

	data->m_cameraLocation.m_v[0] = m_x;
	data->m_cameraLocation.m_v[1] = m_y;
	data->m_cameraLocation.m_v[2] = m_z;

	UpdateD3DView();
#endif
}
void RevCamera::UpdateRendererData(class RevUploadBuffer<struct PassConstants>* passConstant)
{
	PassConstants mMainPassCB = {};
	mMainPassCB.NearZ = 1.0f;
	mMainPassCB.FarZ = 1000.0f;
	mMainPassCB.TotalTime = 0.01f;
	mMainPassCB.DeltaTime = 0.01f;
	mMainPassCB.m_debugValue = GTextureSample;
	
	UINT32 width, height;
	RevEngineFunctions::FindWindowWidthHeight(&width, &height);
	mMainPassCB.m_windowWidth = (float)width;
	mMainPassCB.m_windowHeight = (float)height;

#if !USE_D3D_MATH


	RevMatrix view = m_view.Inverse();

	RevMatrix viewProj = view * m_proj;
	RevMatrix invView = view.Inverse();
	RevMatrix invProj = m_proj.Inverse();
	RevMatrix invViewProj = viewProj.Inverse();

	mMainPassCB.View = view.Transpose();
	mMainPassCB.InvView = invView.Transpose();
	mMainPassCB.Proj = m_proj.Transpose();
	mMainPassCB.InvProj = invProj.Transpose();
	mMainPassCB.ViewProj = viewProj.Transpose();
	mMainPassCB.InvViewProj = invViewProj.Transpose();

	mMainPassCB.EyePosW = RevVector3(&m_view.GetLocation().m_v[0]);

	{
		float halfHeight = mMainPassCB.FarZ* tanf(0.5f*((FOV) * REV_ANGLE_TO_RADIAN));
		float halfWidth = RevEngineFunctions::GetAspectRatio()* halfHeight;

		mMainPassCB.m_frustumCorners1 = RevVector3(-halfWidth, -halfHeight, mMainPassCB.FarZ);
		mMainPassCB.m_frustumCorners2 = RevVector3(-halfWidth, +halfHeight, mMainPassCB.FarZ);
		mMainPassCB.m_frustumCorners3 = RevVector3(+halfWidth, +halfHeight, mMainPassCB.FarZ);
		mMainPassCB.m_frustumCorners4 = RevVector3(+halfWidth, -halfHeight, mMainPassCB.FarZ);
	}
	
#else

#if USE_MODIFIED_MATH
	RevMatrix otherView = m_view.Inverse();
	XMMATRIX view = CreateXMMMatrix(m_view.Inverse());
	XMMATRIX proj = CreateXMMMatrix(m_proj);

	RevMatrix completeProduct = m_view.Inverse() * m_proj;
	XMMATRIX viewProj = CreateXMMMatrix(completeProduct);// XMMatrixMultiply(view, proj);
	XMMATRIX invView = CreateXMMMatrix(otherView.Inverse());// XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = CreateXMMMatrix(m_proj.Inverse());// XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = CreateXMMMatrix(completeProduct.Inverse());

#else

	XMMATRIX view = XMLoadFloat4x4(&m_view);
	XMMATRIX proj = XMLoadFloat4x4(&m_proj);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj =  XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

#endif

	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	XMFLOAT3 v = { m_x, m_y, m_z };
	mMainPassCB.EyePosW = v;
	mMainPassCB.NearZ = 1.0f;
	mMainPassCB.FarZ = 1000.0f;


	{
		float halfHeight = mMainPassCB.FarZ* tanf(0.5f*((FOV)* REV_ANGLE_TO_RADIAN));
		float halfWidth = RevEngineFunctions::GetAspectRatio()* halfHeight;

		mMainPassCB.m_frustumCorners1 = XMFLOAT3(-halfWidth, -halfHeight, mMainPassCB.FarZ);
		mMainPassCB.m_frustumCorners2 = XMFLOAT3(-halfWidth, +halfHeight, mMainPassCB.FarZ);
		mMainPassCB.m_frustumCorners3 = XMFLOAT3(+halfWidth, +halfHeight, mMainPassCB.FarZ);
		mMainPassCB.m_frustumCorners4 = XMFLOAT3(+halfWidth, -halfHeight, mMainPassCB.FarZ);
	}

#endif


	passConstant->CopyData(0, mMainPassCB);
}

void RevCamera::ResetPosition()
{
	m_x = GCameraDefaultLocaton.X();
	m_y = GCameraDefaultLocaton.Y();
	m_z = GCameraDefaultLocaton.Z();

#if !USE_D3D_MATH
	RevVector3 pos = RevVector3(m_x, m_y, m_z);
	RevVector3 towards = (RevVector3(m_x, m_y, m_z - 1) - pos);
	towards.Normalize();
	m_view = RevCreateLookAt(towards);
	m_view.SetLocation(pos);
#else

	UpdateD3DView();

#endif
}

void RevCamera::UpdateD3DView()
{
#if USE_D3D_MATH && !USE_MODIFIED_MATH
	XMVECTOR pos = XMVectorSet(m_x, m_y, m_z, 1.0f);
	XMVECTOR towards = XMVectorSet(m_x, m_y, m_z + 1, 1.0f);

	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMMATRIX viewMatrix = XMMatrixLookAtLH(pos, towards, up);
	XMStoreFloat4x4(&m_view, viewMatrix);
#endif
}
