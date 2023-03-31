#pragma once

class RevPaths
{
public:
	static std::string GetContentPath();
	static std::wstring GetContentPathW();
	static const char* GetContentPathCStr();

	static std::string GetLevelPath();
	static std::string AddLevelPath(const std::string& inPath);

	static std::string AddContentPath(const std::string& inPath);
	static std::wstring AddContentPathWChar(const std::wstring& inPath);
};


