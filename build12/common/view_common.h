
#pragma once


#ifdef DLL_RECONSTRUCTION

#ifdef DLL_RECONSTRUCTION_EXPORTS
#define DLL_RECONSTRUCTION_API __declspec(dllexport)
#else
#define DLL_RECONSTRUCTION_API __declspec(dllimport)
#endif

#else
#define DLL_RECONSTRUCTION_API
#endif

#include <string>

extern "C" DLL_RECONSTRUCTION_API void reset_view();

extern "C" DLL_RECONSTRUCTION_API int render3DResult(char* pInputImageDir, char* pOutputImageDir,
	char* pFilenameSparse, char* pFilenameDense, bool bLogInitialized, 
	char* pColorSky, char* pColorPoint, bool bSameColor, int nSizePoint, 
	char* pOutputFormat, char* pOutputName, int nFPS, int nTimeLength,
	int nWindowWidth, int nWindowHeight);
