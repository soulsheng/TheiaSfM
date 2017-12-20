
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

#include <glog/logging.h>
#include <string>
#include <theia/theia.h>

DLL_RECONSTRUCTION_API void reset_view();

DLL_RECONSTRUCTION_API void render3DResult(std::string &exePath, std::string& ply_file, std::string outputImageDir,
	std::string& pmvsPath, std::string& inputImageDir);
