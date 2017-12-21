
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

DLL_RECONSTRUCTION_API void reset_view();

DLL_RECONSTRUCTION_API void render3DResult(char* pInputImageDir, char* pOutputImageDir,
	char* pFilenameSparse, char* pFilenameDense, bool bLogInitialized);
