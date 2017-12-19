
#pragma once

#include <glog/logging.h>
#include <string>
#include <theia/theia.h>

void reset_view();

void render3DResult(std::string &exePath, std::string& ply_file, std::string outputImageDir,
	std::string& pmvsPath, std::string& inputImageDir);
