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
#include <gflags/gflags.h>
#include <theia/theia.h>
#include <string>
#include <vector>

#include "build_common.h"
#include "view_common.h"

#include <Shellapi.h>

#include "theia/io/read_ply_file.h"

typedef std::string	String;


DEFINE_bool(undistort, false, "bool on/off to undistort image. eg:0 ");
DEFINE_string(eye_position, "(0,0,0)", "position of eye.");
DEFINE_string(eye_angle, "(0,0,0)", "angle of eye.");
DEFINE_bool(build, true, "bool on/off to build. eg:0 ");
#define FLAG_FILE_NAME	"build_reconstruction_flags.txt"

#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

#if 1
String FLAGS_pmvs_working_directory;
String FLAGS_ply_file;
#else
DEFINE_string(pmvs_working_directory, "",
	"A directory to store the necessary pmvs files.");
DEFINE_string(ply_file, "option-0000.ply", "Output PLY file.");
#endif
void prepare_points_to_draw(Reconstruction *reconstruction)
{
	// Centers the reconstruction based on the absolute deviation of 3D points.
	reconstruction->Normalize();

	// Set up camera drawing.
	cameras.reserve(reconstruction->NumViews());
	for (const theia::ViewId view_id : reconstruction->ViewIds()) {
		const auto* view = reconstruction->View(view_id);
		if (view == nullptr || !view->IsEstimated()) {
			continue;
		}
		cameras.emplace_back(view->Camera());
	}

	// Set up world points and colors.
	world_points.reserve(reconstruction->NumTracks());
	point_colors.reserve(reconstruction->NumTracks());
	for (const theia::TrackId track_id : reconstruction->TrackIds()) {
		const auto* track = reconstruction->Track(track_id);
		if (track == nullptr || !track->IsEstimated()) {
			continue;
		}
		world_points.emplace_back(track->Point().hnormalized());
		point_colors.emplace_back(track->Color().cast<float>());
		num_views_for_track.emplace_back(track->NumViews());
	}

	//reconstruction.release();
}

// OpenGL camera parameters.
float zoom_default = 0.0;
float zoom = zoom_default;

Eigen::Vector3f eye_position;
Eigen::Vector3f eye_position_default;

// Rotation values for the navigation
Eigen::Vector2f navigation_rotation_default(60.0, 0.0);
Eigen::Vector2f navigation_rotation(navigation_rotation_default);

//int n_fps = 240; // frame per second

Eigen::Vector2i window_position(200, 100);

int		nColorPoint[3];
extern float point_size;

void getColorFromString(std::string str, int * cColor)
{
	std::istringstream in(str);
	char tmp;
	in >> tmp >> cColor[0] >> tmp >> cColor[1] >> tmp >> cColor[2];
}

void build_reconstruction(std::vector<Reconstruction *>& reconstructions)
{
	const ReconstructionBuilderOptions options =
		SetReconstructionBuilderOptions();

	ReconstructionBuilder reconstruction_builder(options, strPathExe);
	// If matches are provided, load matches otherwise load images.
	if (FLAGS_matches_file.size() != 0) {
		AddMatchesToReconstructionBuilder(&reconstruction_builder);
	}
	else if (FLAGS_images.size() != 0) {
		AddImagesToReconstructionBuilder(&reconstruction_builder);
	}
	else {
		LOG(FATAL)
			<< "You must specifiy either images to reconstruct or a match file.";
	}

	CHECK(reconstruction_builder.BuildReconstruction(&reconstructions))
		<< "Could not create a reconstruction.";
}

void gl_draw_points(int argc, char** argv)
{
	// Set up opengl and glut.
	glutInit(&argc, argv);
	glutInitWindowPosition(window_position[0], window_position[1]);
	window_size[0] = FLAGS_window_width;
	window_size[1] = FLAGS_window_height;
	glutInitWindowSize(window_size[0], window_size[1]);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("SDIOI Reconstruction Viewer");

#ifdef _WIN32
	// Set up glew.
	CHECK_EQ(GLEW_OK, glewInit())
		<< "Failed initializing GLEW.";
#endif

	// Set the camera
	gluLookAt(0.0f, 0.0f, -6.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Set sky color
	int cColor[3];
	getColorFromString(std::string(FLAGS_color_sky), cColor);

	glClearColor(cColor[0] / 255.0f, cColor[1] / 255.0f, cColor[2] / 255.0f, 1.0f);

	// Set point color / size
	getColorFromString(std::string(FLAGS_color_point), nColorPoint);
	point_size = FLAGS_point_size;

	// register callbacks
	glutDisplayFunc(RenderScene);
	glutReshapeFunc(ChangeSize);
	glutMouseFunc(MouseButton);
	glutMotionFunc(MouseMove);
	glutKeyboardFunc(Keyboard);
	glutIdleFunc(RenderScene);

	// enter GLUT event processing loop
	glutMainLoop();
}

#include <fstream>  // NOLINT

void CreateDirectoryIfDoesNotExist(const std::string& directory) {
	if (!theia::DirectoryExists(directory)) {
		CHECK(theia::CreateNewDirectory(directory))
			<< "Could not create the directory: " << directory;
	}
}

int WriteCamerasToPMVS(const theia::Reconstruction& reconstruction) {
	const std::string txt_dir = FLAGS_pmvs_working_directory + "/txt";
	CreateDirectoryIfDoesNotExist(txt_dir);
	const std::string visualize_dir = FLAGS_pmvs_working_directory + "/visualize";

	std::vector<std::string> image_files;
	CHECK(theia::GetFilepathsFromWildcard(FLAGS_images, &image_files))
		<< "Could not find images that matched the filepath: " << FLAGS_images
		<< ". NOTE that the ~ filepath is not supported.";
	CHECK_GT(image_files.size(), 0) << "No images found in: " << FLAGS_images;

	// Format for printing eigen matrices.
	const Eigen::IOFormat unaligned(Eigen::StreamPrecision, Eigen::DontAlignCols);

	int current_image_index = 0;
	for (int i = 0; i < image_files.size(); i++) {
		std::string image_name;
		CHECK(theia::GetFilenameFromFilepath(image_files[i], true, &image_name));
		const theia::ViewId view_id = reconstruction.ViewIdFromName(image_name);
		if (view_id == theia::kInvalidViewId) {
			continue;
		}

		LOG(INFO) << "Undistorting image " << image_name;
		const theia::Camera& distorted_camera =
			reconstruction.View(view_id)->Camera();
		theia::Camera undistorted_camera;
		CHECK(theia::UndistortCamera(distorted_camera, &undistorted_camera));

		theia::FloatImage distorted_image(image_files[i]);
		theia::FloatImage undistorted_image;
		if (FLAGS_undistort)
		CHECK(theia::UndistortImage(distorted_camera,
			distorted_image,
			undistorted_camera,
			&undistorted_image));
		else
			undistorted_image = distorted_image;

		LOG(INFO) << "Exporting parameters for image: " << image_name;

		// Copy the image into a jpeg format with the filename in the form of
		// %08d.jpg.
		const std::string new_image_file = theia::StringPrintf(
			"%s/%08d.jpg", visualize_dir.c_str(), current_image_index);
		undistorted_image.Write(new_image_file);

		// Write the camera projection matrix.
		const std::string txt_file = theia::StringPrintf(
			"%s/%08d.txt", txt_dir.c_str(), current_image_index);
		theia::Matrix3x4d projection_matrix;
		undistorted_camera.GetProjectionMatrix(&projection_matrix);
		std::ofstream ofs(txt_file);
		ofs << "CONTOUR" << std::endl;
		ofs << projection_matrix.format(unaligned);
		ofs.close();

		++current_image_index;
	}

	return current_image_index;
}

void WritePMVSOptions(const std::string& working_dir,
	const int num_images) {
	std::ofstream ofs(working_dir + "/pmvs_options.txt");
	ofs << "level 1" << std::endl;
	ofs << "csize 2" << std::endl;
	ofs << "threshold 0.7" << std::endl;
	ofs << "wsize 7" << std::endl;
	ofs << "minImageNum 3" << std::endl;
	ofs << "CPU " << FLAGS_num_threads << std::endl;
	ofs << "setEdge 0" << std::endl;
	ofs << "useBound 0" << std::endl;
	ofs << "useVisData 0" << std::endl;
	ofs << "sequence -1" << std::endl;
	ofs << "timages -1 0 " << num_images << std::endl;
	ofs << "oimages 0" << std::endl;
}

void export_to_pmvs(theia::Reconstruction& reconstruction)
{
	// Set up output directories.
	CreateDirectoryIfDoesNotExist(FLAGS_pmvs_working_directory);
	const std::string visualize_dir = FLAGS_pmvs_working_directory + "/visualize";
	CreateDirectoryIfDoesNotExist(visualize_dir);
	const std::string txt_dir = FLAGS_pmvs_working_directory + "/txt";
	CreateDirectoryIfDoesNotExist(txt_dir);
	const std::string models_dir = FLAGS_pmvs_working_directory + "/models";
	CreateDirectoryIfDoesNotExist(models_dir);

	const int num_cameras = WriteCamerasToPMVS(reconstruction);
	WritePMVSOptions(FLAGS_pmvs_working_directory, num_cameras);

	const std::string lists_file = FLAGS_pmvs_working_directory + "/list.txt";
	const std::string bundle_file =
		FLAGS_pmvs_working_directory + "/bundle.rd.out";
	CHECK(theia::WriteBundlerFiles(reconstruction, lists_file, bundle_file));
}

void lanch_external_bin(String& bin, String& parameter, String& path)
{
	String zipParameter = String("a -m0 -inul -idp -sfxDefault.SFX -ibck -iiconVRGIS.ico -zsescript ");

	SHELLEXECUTEINFO ShExecInfo = { 0 };
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = bin.c_str();
	ShExecInfo.lpParameters = parameter.c_str();
	ShExecInfo.lpDirectory = path.c_str();
	ShExecInfo.nShow = SW_HIDE;
	ShExecInfo.hInstApp = NULL;
	ShellExecuteEx(&ShExecInfo);

	long waitStatus = WaitForSingleObject(ShExecInfo.hProcess, INFINITE);

	if (waitStatus)
		printf("failed \n");
	else
		printf("succeed \n");

}

String getPath(String& strFullPath)
{
	int indexEnd = strFullPath.find_last_of('\\');
	return strFullPath.substr(0, indexEnd);
}

void run_pmvs(char *exeFullPath)
{
	String exePath = getPath(String(exeFullPath));
	lanch_external_bin(String("cmvs.exe"), FLAGS_pmvs_working_directory, exePath);

	lanch_external_bin(String("genOption.exe"), FLAGS_pmvs_working_directory, exePath);

	String parameter = FLAGS_pmvs_working_directory + " option-0000";
	lanch_external_bin(String("pmvs2.exe"), parameter, exePath);

}

void rand_num_views_for_track(std::vector<int>& num_views_for_track, int size)
{
	num_views_for_track.reserve(size);
	for (int i = 0; i < size; i++)
		num_views_for_track.emplace_back(rand() % 10);
}

template<typename T>
void getValueFromString(std::string str, T * cColor)
{
	std::istringstream in(str);
	char tmp;
	in >> tmp >> cColor[0] >> tmp >> cColor[1] >> tmp >> cColor[2];
}

void  calculate(Eigen::Vector3d& minPoint, Eigen::Vector3d& maxPoint, theia::Vector3dVec& allPoints)
{
	for (theia::Vector3dVec::iterator itr = allPoints.begin(); itr != allPoints.end(); itr++)
	{
		double x = itr->x();
		double y = itr->y();
		double z = itr->z();

		if (FLAGS_swap_yz) 
		{
			double tmp = y;
			y = -z;
			z = tmp;

			itr->y() = y;
			itr->z() = z;
		}

		if (x < minPoint.x())	minPoint.x() = x;
		if (y < minPoint.y())	minPoint.y() = y;
		if (z < minPoint.z())	minPoint.z() = z;

		if (x > maxPoint.x())	maxPoint.x() = x;
		if (y > maxPoint.y())	maxPoint.y() = y;
		if (z > maxPoint.z())	maxPoint.z() = z;
	}
}

int main(int argc, char* argv[]) {
  THEIA_GFLAGS_NAMESPACE::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  strPathExe = argv[0];
  strPathExe = strPathExe.substr(0, strPathExe.find_last_of("\\") + 1);

  //google::ReadFromFlagsFile(FLAG_FILE_NAME, strPathExe.c_str(), false);
  //std::cout << "exe path..." << strPathExe << std::endl;
  FLAGS_images = FLAGS_input_images + "*.jpg";
  FLAGS_output_matches_file = FLAGS_input_images + "output.matches";
  FLAGS_output_reconstruction = FLAGS_input_images + "result";
  FLAGS_matching_working_directory = FLAGS_input_images + "features\\";
  FLAGS_pmvs_working_directory = FLAGS_input_images + "pmvs\\";
  FLAGS_ply_file = FLAGS_pmvs_working_directory + "models\\option-0000.ply";

  CreateDirectoryIfDoesNotExist(FLAGS_matching_working_directory);

  if (FLAGS_build)
  {
	  Reconstruction* reconstruction = NULL;

#if 1
	  std::vector<Reconstruction*> reconstructions;

	  build_reconstruction(reconstructions);

	  if (reconstructions.size())
		  reconstruction = reconstructions[0];
	  else
		  return -1;

	  theia::ColorizeReconstruction(FLAGS_input_images,
		  FLAGS_num_threads,
		  reconstruction);

	  theia::WriteReconstruction(*reconstruction,
		  FLAGS_output_reconstruction);

#else
	  reconstruction = new theia::Reconstruction();

	  CHECK(ReadReconstruction(FLAGS_output_reconstruction, reconstruction))
		  << "Could not read reconstruction file.";

#endif

	  // Centers the reconstruction based on the absolute deviation of 3D points.
	  reconstruction->Normalize();

#if 1
	  export_to_pmvs(*reconstruction);
#endif

	  //prepare_points_to_draw( reconstruction );

#if 1
	  run_pmvs(argv[0]);
#endif

  }// if (FLAGS_build)

  //if (FLAGS_view)
  {
	  clock_t tBegin = clock();
	  //std::cout << "ReadPlyFile begin: " << tBegin << std::endl;
	  if (!theia::ReadPlyFile(FLAGS_ply_file, world_points, point_normals, point_colors))
		  printf("can not open ply file!\n");
	  //std::cout << "ReadPlyFile cost " << (clock() - tBegin) / 1000 << " seconds" << std::endl;

	  rand_num_views_for_track(num_views_for_track, world_points.size());

	  float fEyePosition[3];
	  getValueFromString(std::string(FLAGS_eye_position), fEyePosition);
	  float fEyeAngle[3];
	  getValueFromString(std::string(FLAGS_eye_angle), fEyeAngle);

	  calculate(minPoint, maxPoint, world_points);

	  Eigen::Vector3d midPoint = (maxPoint + minPoint) / 2;
	  Eigen::Vector3d sizeRect = maxPoint - minPoint;

	  double lengthMax = sizeRect.x() > sizeRect.z() ? sizeRect.x() : sizeRect.z();

	  fEyePosition[0] += midPoint.x();
	  fEyePosition[1] += midPoint.y();
	  fEyePosition[2] += midPoint.z() + lengthMax*FLAGS_distance;

	  setEyeParameter(fEyePosition, fEyeAngle);

	  gl_draw_points(argc, argv);
  }

  return 0;
}
