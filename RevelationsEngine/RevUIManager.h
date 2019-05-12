#pragma once

class RevUIManager
{
public:
	RevUIManager()
	{
	}

	void Initialize(void* hwnd);
	void Draw();

	void DrawGameWindow();

	void CopySRV(struct ID3D12Resource* resource);

	
private:

	void InitializeShaderInputLayout();
	void InitializeRootSignature();
	void InitializeUIIO(void* hwnd);

	

};