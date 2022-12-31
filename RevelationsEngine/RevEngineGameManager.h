#pragma once

#include "RevEngineManager.h"

class RevEngineGameManager : public RevEngineManager
{
public:
	RevEngineGameManager()
	{
		m_isImguiEnabled = false;
	}
};
