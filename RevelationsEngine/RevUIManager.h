#pragma once
#include "RevUITypes.h"

struct ImFont;

class RevUIManager
{
public:
	RevUIManager(){}
	~RevUIManager();

	static RevFont* FindFontFromType(RevFontType type);
	
	void Initialize(void* hwnd);
	void Draw();

	void DrawGameWindow();
	void CopySRV(struct ID3D12Resource* resource);
	
private:

	void InitializeShaderInputLayout();
	void InitializeRootSignature();
	void InitializeUIIO(void* hwnd);

	RevFont* ConstructFont(const char* fileName, RevFontType type);

	std::vector<RevFont*> m_fonts;


};