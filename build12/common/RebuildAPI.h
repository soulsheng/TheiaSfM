
#pragma once 

#include <string>
#include <vector>

void kernelReBuildSparse(std::string &exePath, std::string& inputImageDir);

void kernelReBuildDense(std::string &exePath, std::string& pmvsPath, std::string& ply_file, std::string& inputImageDir);
