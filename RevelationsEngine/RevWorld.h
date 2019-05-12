#pragma once

#include <vector>
#include "RevUploadBuffer.h"
#include "DirectXMath.h"
#include "RevInstance.h"
#include <string>

class RevWorld
{
public:
	void DestoyWorld();
	 
	void Draw(struct RevModelFrameRender& render);
	void UpdateRendererData(struct RevFrameResource* resource, float deltaTime);
	void ReplaceInstance(UINT index, RevInstance* newInstance);

	std::vector<class RevInstance*> m_instances;
	std::vector<struct RevLight*> m_lights;
	std::string m_currentWorldPath;
};