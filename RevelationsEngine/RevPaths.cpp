#include "stdafx.h"
#include "RevPaths.h"

#define CONCATENATE(e1, e2) e1 ## e2
#define PREFIX_L(s) CONCATENATE(L, s)

#define REV_CONTENT_PATH "..\\..\\Content\\"
#define REV_CONTENT_PATH_W(s) CONCATENATE(L, s)
#define REV_LEVEL_PATH "..\\..\\Content\\Data\\Level\\"

std::string RevPaths::GetContentPath()
{
	return REV_CONTENT_PATH;
}

std::wstring RevPaths::GetContentPathW()
{
	return PREFIX_L(REV_CONTENT_PATH);
}

const char* RevPaths::GetContentPathCStr()
{
	return REV_CONTENT_PATH;
}

std::string RevPaths::GetLevelPath()
{
	return REV_LEVEL_PATH;
}

std::string RevPaths::AddLevelPath(const std::string& inPath)
{
	std::string baseLevelPath = GetLevelPath();
	return baseLevelPath.append(inPath);
}

std::string RevPaths::AddContentPath(const std::string& inPath)
{
	std::string baseContentPath = GetContentPath();
	return baseContentPath.append(inPath);
}

std::wstring RevPaths::AddContentPathWChar(const std::wstring& inPath)
{
	std::wstring baseContentPath = GetContentPathW();
	return baseContentPath.append(inPath);
}
