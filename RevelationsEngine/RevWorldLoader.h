#pragma once

#include <vector>

struct RevGameObjectType
{
	char m_name[512];
	char m_instancePath[512];

	RevGameObjectType()
	{
		memset(this, 0, sizeof(RevGameObjectType));
	}
};

class RevWorldLoader
{
public:
	void Initialize();

	class RevWorld* LoadWorld(const char* fileName);
	void SpawnInstanceToWorld(class RevWorld* world, const char* name);

	std::vector< RevGameObjectType> m_objectTypes;
};