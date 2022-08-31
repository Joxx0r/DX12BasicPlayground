#pragma once

#include <vector>
#include <string>


#define AMOUNT_OF_MODELS_ALLOC 500

struct RevModelEntry
{
	class RevModel* m_model = nullptr;
	std::string m_name;
	UINT m_handle = INT32_MAX;
	UINT32 m_reserved : 1;

	bool operator==(UINT handle)
	{
		return m_handle == handle;
	}
	bool operator==(std::string name)
	{
		return m_name == name;
	}
};

class RevModelManager
{
public:
	RevModelManager() { m_models = nullptr; };
	void Initialize();

	static RevModelEntry* FindModel(const char* fileName);
	static RevModel* FindModelByHandle(UINT handle);

protected:
	
	RevModelEntry* FindOrCreateModelByPathInternal(const char* fileName);

public:
	
	RevModelEntry* m_models;

	static UINT GModelCounter;
};