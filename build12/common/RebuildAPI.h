
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

extern "C" DLL_RECONSTRUCTION_API int kernelReBuildSparse(char* pInputImageDir, char* filename_sparse);

extern "C" DLL_RECONSTRUCTION_API int kernelReBuildDense(char* pInputImageDir, char* filename_sparse, char* filename_dense, bool isLogInitialized);

extern "C" DLL_RECONSTRUCTION_API std::string get_Path(std::string& strFullPath);

extern "C" DLL_RECONSTRUCTION_API bool lanch_external(std::string& bin, std::string& parameter, std::string& path, int nShowType = 0/*SW_HIDE*/);

extern "C" DLL_RECONSTRUCTION_API std::string get_EXEDLLPath();

void SetLog(std::string &exePath);