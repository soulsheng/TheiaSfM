
#ifndef UTILITY_COMMON_H
#define UTILITY_COMMON_H

#include <string>
#include <vector>

std::string getPath(std::string& strFullPath);

void getPath(const char* strFullPath, char* strPath);

void resizeImageFiles(std::vector<std::string>& image_files, int nResize, bool bForceResize );

std::string getEXEDLLPath();
std::string getEXEDLLFullPath();

bool fileExist(std::string& filename);

#endif // UTILITY_COMMON_H