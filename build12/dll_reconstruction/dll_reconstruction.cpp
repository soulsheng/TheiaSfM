// dll_reconstruction.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ
// Windows ͷ�ļ�: 
#include <windows.h>

#include "dll_reconstruction.h"


// ���ǵ���������һ��ʾ��
DLL_RECONSTRUCTION_API int ndll_reconstruction=0;

// ���ǵ���������һ��ʾ����
DLL_RECONSTRUCTION_API int fndll_reconstruction(void)
{
	return 42;
}

// �����ѵ�����Ĺ��캯����
// �й��ඨ�����Ϣ������� dll_reconstruction.h
Cdll_reconstruction::Cdll_reconstruction()
{
	return;
}
