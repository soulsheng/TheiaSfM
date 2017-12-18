
#pragma once 

#include <string>
#include <vector>

void kernelReBuildSparse(std::string &exePath, std::string& inputImageDir);

void kernelReBuildDense(std::string& pmvsPath, std::string& ply_file, std::string& inputImageDir);

void render3DResult(std::string &exePath, std::string& ply_file, std::string outputImageDir, 
	std::string& pmvsPath, std::string& inputImageDir);