// Copyright (C) 2014 The Regents of the University of California (Regents).
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//
//     * Neither the name of The Regents or University of California nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Please contact the author of this library if you have any questions.
// Author: Chris Sweeney (cmsweeney@cs.ucsb.edu)

#include <Eigen/Core>
#include <glog/logging.h>
//#include <gflags/gflags.h>
#include <theia/theia.h>
#include <string>
#include <vector>

#include "build_common.h"
#include "view_common.h"


#include "theia/io/read_ply_file.h"

#include "utility_theia.h"

#include "LaunchPMVS2.h"

typedef std::string	String;


DEFINE_bool(undistort, false, "bool on/off to undistort image. eg:0 ");

DEFINE_bool(build, true, "bool on/off to build. eg:0 ");

DEFINE_bool(build_sparse, true, "bool on/off to build. eg:0 ");

DEFINE_int32(threshold_group, 35, "threshodGroup to filter group of outlier points.");

#define FLAG_FILE_NAME	"build_reconstruction_flags.txt"

#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

#if 1
String FLAGS_ply_file;
#else
DEFINE_string(pmvs_working_directory, "",
	"A directory to store the necessary pmvs files.");
DEFINE_string(ply_file, "option-0000.ply", "Output PLY file.");
#endif
Reconstruction* current_reconstruction=NULL;

//Eigen::Vector3f eye_position;
//Eigen::Vector3f eye_position_default;

// Rotation values for the navigation
//Eigen::Vector2f navigation_rotation_default(60.0, 0.0);
//Eigen::Vector2f navigation_rotation(navigation_rotation_default);

//int n_fps = 240; // frame per second



#include <fstream>  // NOLINT

void rand_num_views_for_track(std::vector<int>& num_views_for_track, int size)
{
	num_views_for_track.reserve(size);
	for (int i = 0; i < size; i++)
		num_views_for_track.emplace_back(rand() % min_num_views_for_track);
}

void	convertSparseToDense()
{
	if (!FLAGS_build)
		return;

	if (0 == current_reconstruction->NumViews())
		return;

#if 1
	export_to_pmvs(*current_reconstruction, FLAGS_pmvs_working_directory, FLAGS_undistort);
#endif


	LOG(INFO) << "开始执行稠密重建：";
#if 1
	run_pmvs(strPathExe.c_str(), FLAGS_pmvs_working_directory, FLAGS_threshold_group);
#endif
	LOG(INFO) << "执行稠密重建完成！";

}

void	viewDenseResult()
{
	world_points.clear();
	point_normals.clear();
	point_colors.clear();

	clock_t tBegin = clock();
	//std::cout << "ReadPlyFile begin: " << tBegin << std::endl;
	if (!theia::ReadPlyFile(FLAGS_ply_file, world_points, point_normals, point_colors))
		printf("can not open ply file!\n");
	//std::cout << "ReadPlyFile cost " << (clock() - tBegin) / 1000 << " seconds" << std::endl;

	rand_num_views_for_track(num_views_for_track, world_points.size());

	box.calculate(world_points, FLAGS_swap_yz);

	FLAGS_view_type = VIEW_PERSPECTIVE;

	setDefaultCameraProperty();

	LOG(INFO) << "稠密重建三维点的数目为：" << world_points.size();
	LOG(INFO) << "输出稠密重建结果！";
}

void kernelReBuild(std::string &exePath, std::string inputImageDir)
{
	//THEIA_GFLAGS_NAMESPACE::ParseCommandLineFlags(&argc, &argv, true);
	google::InitGoogleLogging(exePath.c_str());


	google::SetLogDestination(google::GLOG_INFO, exePath.c_str());
  google::SetLogDestination(google::GLOG_ERROR, "");
  google::SetLogDestination(google::GLOG_WARNING, "");
  google::SetLogFilenameExtension(".log");

  std::string logFilename = std::string(exePath.c_str()) + ".log";
  if (theia::FileExists(logFilename))
	unlink(logFilename.c_str());

  strPathExe = exePath.c_str();
  strPathExe = strPathExe.substr(0, strPathExe.find_last_of("\\") + 1);

  //google::ReadFromFlagsFile(FLAG_FILE_NAME, strPathExe.c_str(), false);
  //std::cout << "exe path..." << strPathExe << std::endl;
  FLAGS_images = inputImageDir + "*.jpg";
  FLAGS_output_matches_file = inputImageDir + "output.matches";
  FLAGS_output_reconstruction = inputImageDir + "result";
  FLAGS_matching_working_directory = inputImageDir + "features\\";
  FLAGS_pmvs_working_directory = inputImageDir + "pmvs\\";
  FLAGS_ply_file = FLAGS_pmvs_working_directory + "models\\option-0000.ply";

  //############### Logging Options ###############
  //  # Logging verbosity.
  FLAGS_logtostderr = false;
  // # Increase this number to get more verbose logging.
  FLAGS_v = 1;

  min_num_views_for_track = FLAGS_fps * FLAGS_length;

  ReCreateDirectory(FLAGS_matching_working_directory);

  Reconstruction* reconstruction = NULL;
  if (FLAGS_build_sparse && FLAGS_build)
  {
	  build_reconstruction(reconstruction, strPathExe, inputImageDir);
  }
  else
  {
	  reconstruction = new theia::Reconstruction();

	  CHECK(ReadReconstruction(FLAGS_output_reconstruction, reconstruction))
		  << "Could not read reconstruction file.";

  }

  current_reconstruction = reconstruction;

	  // view sparse 
	  prepare_points_to_draw(reconstruction);

  if (FLAGS_view_sparse)
  {

	  min_num_views_for_track = 0;

	  box.calculate(world_points);

	  FLAGS_view_type = VIEW_CAMERA;

	  setDefaultCameraProperty();
  }
  else
  {
	  convertSparseToDense();
	  viewDenseResult();
  }
}

int main(int argc, char* argv[]) {

	if (argc < 3)
		return -1;

	std::string exePath = argv[0];
	std::string inputImageDir = argv[1];
	std::string outputImageDir = argv[2];
	
	kernelReBuild(exePath, inputImageDir);

  gl_draw_points(1, (char*)exePath.c_str(), outputImageDir);

  return 0;
}
