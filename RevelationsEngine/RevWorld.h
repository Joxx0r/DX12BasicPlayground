#pragma once

#include <vector>

class RevGameState;
class RevObject;
class RevPhysxWorld;

class RevWorld
{
public:
	RevWorld();
	void Initialize();
	 
	void Draw(struct RevModelFrameRender& render);
	void UpdateRendererData(struct RevFrameResource* resource, float deltaTime);
	void DestoyWorld();
	
	RevGameState* m_revGameState;
	RevPhysxWorld* m_physicsWorld;
	std::vector<struct RevLight*> m_lights;
};
