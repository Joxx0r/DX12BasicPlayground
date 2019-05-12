#pragma once

#include <vector>
#include <string>


#define AMOUNT_OF_MODELS_ALLOC 500

struct RevModelEntry
{
	class RevModel* m_model = nullptr;
	std::string m_name;
	UINT32 m_reserved : 1;
};

class RevModelManager
{
public:
	RevModelManager() { m_models = nullptr; };
	void Initialize();

	static RevModelEntry* FindModel(const char* fileName);

	RevModelEntry* FindOrCreateModel(const char* fileName);

	RevModelEntry* m_models;

};