#pragma once

#include <vector>

struct RevGameObjectType
{
	std::string m_name;
	std::string m_instancePath;

	RevGameObjectType()
	{
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