#pragma once
#include "stdafx.h"
#include "RevArchive.h"

#pragma optimize("", off )

void RevArchiveLoader::Serialize(void* inputData, UINT size)
{
	assert(inputData != nullptr);
	unsigned char* bufferData = GetMemory(size);
	assert(bufferData != nullptr);
	memcpy(inputData, bufferData, size);
}


void RevArchiveLoader::SerializeBinaryData(void** inputData, UINT size)
{
	*inputData = malloc(size);
	Serialize(*inputData, size);
}

void RevArchiveSaver::Serialize(void* inputData, UINT size)
{
	assert(inputData != nullptr);
	unsigned char* bufferData = GetMemory(size);
	assert(bufferData != nullptr);
	memcpy(bufferData, inputData, size);
}

void RevArchiveSaver::SerializeBinaryData(void** inputData, UINT size)
{
	Serialize(*inputData, size);
}

#pragma optimize("", on)