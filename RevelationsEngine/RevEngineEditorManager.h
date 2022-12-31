#pragma once

#include "RevEngineManager.h"

class RevEngineEditorManager : public RevEngineManager
{
public:
	RevEngineEditorManager()
	{
	}

	virtual void Initialize() override;

protected:

	void UpdateInternal(float deltaTime, void* hwnd) override;


};