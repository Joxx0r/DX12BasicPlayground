// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <assert.h>

#include <d3d12.h>
#include <d3dx12.h>
#include <stdio.h>
#include <stdint.h>

#include "RevTypes.h"
#include "RevEngineFunctions.h"
#include "RevDefines.h"

#define USE_ASSIMP 0
#define DO_LOAING_BINARY 1
#define USE_IMGUI 1

#define REV_PI 3.1415927f
#define REV_INV_PI 1 / REV_PI
#define REV_ANGLE_TO_RADIAN REV_PI / 180
#define REV_RADIAN_TO_ANGLE 180 / REV_PI 


#ifndef RevThrowIfFailed
#define RevThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    if(FAILED(hr__))													\
	{  \
		char buffer[5000] = { '\0' };	\
		sprintf(buffer, "return code: %i %s %i", (int)hr__, __FILE__, __LINE__); \
		DEBUG_ASSERT(0 && buffer); \
	} \
}
#endif

// TODO: reference additional headers your program requires here
