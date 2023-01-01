#pragma once

class RevMovementFunctions
{
public:
	static RevVector3 CalculateMovementChangeBasedOnInput(
		const RevMatrix& startTransform,
		float movementSpeed,
		float deltaTime,
		const RevInputData& inputData);


	static RevInputData CalculateCurrentInput();
};


