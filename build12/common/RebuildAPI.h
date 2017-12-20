
#pragma once 

#include <string>
#include <vector>

#ifdef DLL_RECONSTRUCTION

#ifdef DLL_RECONSTRUCTION_EXPORTS
#define DLL_RECONSTRUCTION_API __declspec(dllexport)
#else
#define DLL_RECONSTRUCTION_API __declspec(dllimport)
#endif

#else
#define DLL_RECONSTRUCTION_API
#endif

extern "C" DLL_RECONSTRUCTION_API void kernelReBuildSparse(std::string &exePath, std::string& inputImageDir);

extern "C" DLL_RECONSTRUCTION_API void kernelReBuildDense(std::string &exePath, std::string& pmvsPath, std::string& ply_file, std::string& inputImageDir);

extern "C" DLL_RECONSTRUCTION_API std::string get_Path(std::string& strFullPath);

extern "C" DLL_RECONSTRUCTION_API bool lanch_external(std::string& bin, std::string& parameter, std::string& path, int nShowType = 0/*SW_HIDE*/);
