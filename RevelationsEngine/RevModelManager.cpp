#include "stdafx.h"
#include "RevModelManager.h"
#include "RevModelLoader.h"

uint32_t RevModelManager::GModelCounter = 0;

void RevModelManager::Initialize()
{
	//do some implementions later
	const int sizeOfModels = sizeof(RevModelEntry) * AMOUNT_OF_MODELS_ALLOC;
	m_models = (RevModelEntry*)malloc(sizeOfModels);
	memset(m_models, 0, sizeOfModels);
}

RevModelEntry* RevModelManager::FindModel(const char* fileName)
{
	return RevEngineFunctions::FindModelManager()->FindOrCreateModelByPathInternal(fileName);
}

RevModel* RevModelManager::FindModelByHandle(uint32_t handle)
{
	if(RevModelManager* manager = RevEngineFunctions::FindModelManager())
	{
		uint32_t modelindex = 0;
		for (modelindex; modelindex < AMOUNT_OF_MODELS_ALLOC; modelindex++)
		{
			RevModelEntry* model = &manager->m_models[modelindex];
			if (model->m_handle == handle)
			{
				return model->m_model;
			}
		}
	}
	return nullptr;
}

RevModelEntry* RevModelManager::FindOrCreateModelByPathInternal(const char* fileName)
{
	uint32_t modelindex = 0;
	for (modelindex; modelindex < AMOUNT_OF_MODELS_ALLOC; modelindex++)
	{
		RevModelEntry* model = &m_models[modelindex];
		if (!model->m_reserved)
		{
			//we do not have this model reserved so will create new one
			break;
		}
		if (model->m_name == fileName)
		{
			if (!model->m_model)
			{
				model->m_model = RevModelLoader::LoadModel(fileName);
			}
			return model;
		}
	}

	if (modelindex >= AMOUNT_OF_MODELS_ALLOC)
	{
		REV_ASSERT(0 && "Increase amount of models loaded");
	}

	RevModelEntry model = {};
	model.m_handle = ++RevModelManager::GModelCounter;
	model.m_name = fileName;
	model.m_reserved = true;
	model.m_model = RevModelLoader::LoadModel(fileName);
	m_models[modelindex] = model;
	return &m_models[modelindex];
}
