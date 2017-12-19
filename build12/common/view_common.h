
#pragma once

#include <glog/logging.h>
#include <string>
#include <theia/theia.h>

void reset_view();

void gl_draw_points(int argc, char* argv, std::string& output_images,
	std::string& pmvsPath, std::string& ply_file, std::string& inputImageDir, std::string &exePath);


void render3DResult(std::string &exePath, std::string& ply_file, std::string outputImageDir,
	std::string& pmvsPath, std::string& inputImageDir);

void	viewDenseResult(std::string& ply_file);

void rand_num_views_for_track(std::vector<int>& num_views_for_track, int size);

void compressBMP(std::string& strFormat, int nImageCountOutput, std::string& strOutput,
	std::string& strPathExe, std::string& outputName, int fps, int width, int height);
