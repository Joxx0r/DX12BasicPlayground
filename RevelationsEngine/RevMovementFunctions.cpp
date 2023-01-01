#include "stdafx.h"
#include "RevMovementFunctions.h"
#include "RevInputManager.h"


RevVector3 RevMovementFunctions::CalculateMovementChangeBasedOnInput(
	const RevMatrix& startTransform,
	float movementSpeed,
	float deltaTime,
	const RevInputData& inputData)
{
	RevVector3 forward = startTransform.GetAxis(RevMatrixAxis::Forward);
	RevVector3 right = startTransform.GetAxis(RevMatrixAxis::Right);
	RevVector3 up = startTransform.GetAxis(RevMatrixAxis::Up);

	RevVector3 currentLocation = startTransform.GetLocation();
	if (RevInputManager::IsInputEnabled(EYInputType::Game))
	{
		if (inputData.m_leftButtonActive)
		{
			currentLocation += -right * movementSpeed * deltaTime;
		}

		if (inputData.m_rightButtonActive)
		{
			currentLocation += right * movementSpeed * deltaTime;
		}

		if (inputData.m_upButtonActive)
		{
			currentLocation += -up * movementSpeed * deltaTime;
		}

		if (inputData.m_downButtonActive)
		{
			currentLocation += up * movementSpeed * deltaTime;
		}

		if (inputData.m_forwardButtonActive)
		{
			currentLocation += forward * movementSpeed * deltaTime;
		}

		if (inputData.m_backwardsButtonActive)
		{
			currentLocation += -forward * movementSpeed * deltaTime;
		}
	}

	return currentLocation;
}

RevInputData RevMovementFunctions::CalculateCurrentInput()
{
	RevInputData data = {};
	data.m_leftButtonActive = (GetKeyState(REV_VK_A) >> 15) & 1;
	data.m_rightButtonActive = (GetKeyState(REV_VK_D) >> 15) & 1;
	data.m_upButtonActive = (GetKeyState(REV_VK_Q) >> 15) & 1;
	data.m_downButtonActive = (GetKeyState(REV_VK_E) >> 15) & 1;
	data.m_backwardsButtonActive = (GetKeyState(REV_VK_S) >> 15) & 1;
	data.m_forwardButtonActive = (GetKeyState(REV_VK_W) >> 15) & 1;
	return data;
}
