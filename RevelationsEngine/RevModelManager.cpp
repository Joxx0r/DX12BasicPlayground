#include "stdafx.h"
#include "RevModelManager.h"
#include "RevUtils.h"
#include "RevModel.h"
#include "RevModelManager.h"
#include "RevModelLoader.h"

void RevModelManager::Initialize()
{
	//do some implementions later
	const int sizeOfModels = sizeof(RevModelEntry) * AMOUNT_OF_MODELS_ALLOC;
	m_models = (RevModelEntry*)malloc(sizeOfModels);
	memset(m_models, 0, sizeOfModels);
}

RevModelEntry* RevModelManager::FindOrCreateModel(const char* fileName)
{
	UINT modelindex = 0;
	for (modelindex; modelindex < AMOUNT_OF_MODELS_ALLOC; modelindex++)
	{
		RevModelEntry* model = &m_models[modelindex];
		if (!model->m_reserved)
		{
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
		assert(0 && "Increase amount of models loaded");
	}

	RevModelEntry model = {};
	model.m_name = fileName;
	model.m_reserved = true;
	model.m_model = RevModelLoader::LoadModel(fileName);
	m_models[modelindex] = model;
	return &m_models[modelindex];
}

RevModelEntry* RevModelManager::FindModel(const char* fileName)
{
	return RevEngineFunctions::FindModelManager()->FindOrCreateModel(fileName);
}

