
#include "stdafx.h"
#include "utility_common.h"
#include <string.h>
#include <stdlib.h>  

std::string getPath(std::string& strFullPath)
{
	int indexEnd = strFullPath.find_last_of('\\');
	return strFullPath.substr(0, indexEnd + 1);
}

void getPath(const char* strFullPath, char* pPath)
{
	std::string strPath(strFullPath);
	strPath = getPath(strPath);
	strcpy_s(pPath, sizeof(pPath), strPath.c_str());
	pPath[strPath.size()] = '\0';
}
