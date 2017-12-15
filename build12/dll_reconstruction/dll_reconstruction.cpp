// dll_reconstruction.cpp : 定义 DLL 应用程序的导出函数。
//

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件: 
#include <windows.h>

#include "dll_reconstruction.h"


// 这是导出变量的一个示例
DLL_RECONSTRUCTION_API int ndll_reconstruction=0;

// 这是导出函数的一个示例。
DLL_RECONSTRUCTION_API int fndll_reconstruction(void)
{
	return 42;
}

// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 dll_reconstruction.h
Cdll_reconstruction::Cdll_reconstruction()
{
	return;
}
