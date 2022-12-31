#include "stdafx.h"
#include "RevShadermanager.h"
#include "RevUtils.h"

#define AMOUNT_OF_SHADER_MALLOC 100

void RevShaderManager::Initialize()
{
	DEBUG_ASSERT(m_shaders == nullptr);
	//do some implementions later
	const int sizeofShader = sizeof(RevShader) * AMOUNT_OF_SHADER_MALLOC;
	m_shaders = (RevShader*)malloc(sizeofShader);
	memset(m_shaders, 0, sizeofShader);
}

RevShader* RevShaderManager::GetShaderInternal(const RevShaderRequestData& data)
{
	int shaderIndex = 0;
	for (; shaderIndex < AMOUNT_OF_SHADER_MALLOC; shaderIndex++)
	{
		RevShader shader = m_shaders[shaderIndex];
		if (!shader.m_initialized)
		{
			break;
		}

		if (shader.m_name == data.m_fileName)
		{
			return &m_shaders[shaderIndex];
		}
	}
	if (shaderIndex >= AMOUNT_OF_SHADER_MALLOC)
	{
		DEBUG_ASSERT(0 && "Increase amount of shaders required");
	}

	RevShader shader = {};
	shader.m_name = data.m_fileName;
	shader.m_initialized = true;
	{
		shader.m_vsByteCode = RevUtils::CompileVertexShader(data.m_fileName);
	}

	if(data.m_compilePixelShader)
	{
		shader.m_psByteCode = RevUtils::CompilePixelShader(data.m_fileName);
	}

	m_shaders[shaderIndex] = shader;
	return &m_shaders[shaderIndex];
}

RevShader* RevShaderManager::GetShader(const wchar_t* fileName, bool compilePixelShader /*= true*/)
{
	RevShaderRequestData newRequestData = { };
	newRequestData.m_fileName = const_cast<wchar_t*>(fileName);
	newRequestData.m_compilePixelShader = compilePixelShader;
	return RevEngineFunctions::FindRevShaderManager()->GetShaderInternal(newRequestData);
}

