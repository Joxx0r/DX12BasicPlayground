#pragma once

#include <vector>
#include <string>

struct RevShader
{
	ID3DBlob* m_vsByteCode;
	ID3DBlob* m_psByteCode;
	std::wstring m_name;
	uint32_t m_initialized : 1;
};

struct RevShaderRequestData
{
	wchar_t* m_fileName;
	bool m_compilePixelShader = true;
};

class RevShaderManager
{
public:
	RevShaderManager() { m_shaders = nullptr;  };
	void Initialize();

	static RevShader* GetShader(const wchar_t* fileName, bool compilePixelShader = true);

	RevShader* GetShaderInternal(const RevShaderRequestData& data);

	RevShader* m_shaders;

};