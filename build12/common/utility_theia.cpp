
#include "utility_theia.h"


#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/types_c.h>  // include it before #include <OpenImageIO/imagebufalgo.h>
#include <opencv2/highgui/highgui_c.h>

#include "LaunchPMVS2.h"
#include <iostream> 
#include <fstream>
#include <iomanip>
#include <stlplus3/file_system.hpp>
#include "theia/util/filesystem.h"

#include "build_common.h"

//using theia::Reconstruction;
//using theia::ReconstructionBuilder;
//using theia::ReconstructionBuilderOptions;

void CreateDirectoryIfDoesNotExist(const std::string& directory) {
	if (!theia::DirectoryExists(directory)) {
		CHECK(theia::CreateNewDirectory(directory))
			<< "Could not create the directory: " << directory;
	}
}

void ReCreateDirectory(const std::string& directory) {
	if (theia::DirectoryExists(directory))
		theia::DeleteDirectory(directory);

	CHECK(theia::CreateNewDirectory(directory))
		<< "Could not create the directory: " << directory;
}

int WriteCamerasToPMVS(const theia::Reconstruction& reconstruction, std::string& pmvsPath, bool undistort, 
	std::string& FLAGS_images) {
	const std::string txt_dir = pmvsPath + "/txt";
	CreateDirectoryIfDoesNotExist(txt_dir);
	const std::string visualize_dir = pmvsPath + "/visualize";

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

		//LOG(INFO) << "Undistorting image " << image_name;
		const theia::Camera& distorted_camera =
			reconstruction.View(view_id)->Camera();
		theia::Camera undistorted_camera(distorted_camera);

		theia::FloatImage distorted_image(image_files[i]);
		theia::FloatImage undistorted_image(distorted_image);
		if (undistort)
		{
			CHECK(theia::UndistortCamera(distorted_camera, &undistorted_camera));
			CHECK(theia::UndistortImage(distorted_camera,
				distorted_image,
				undistorted_camera,
				&undistorted_image));
		}

		//LOG(INFO) << "Exporting parameters for image: " << image_name;

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
	const int num_images, const int FLAGS_num_threads) {
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

bool export_to_pmvs(std::string& pmvsPath, std::string& inputImageDir, std::string& filenameSparse,
	const int FLAGS_num_threads, bool undistort)
{
	theia::Reconstruction reconstruction;

	CHECK(ReadReconstruction(filenameSparse, &reconstruction))
		<< "Could not read reconstruction file.";

	if (0 == reconstruction.NumViews())
		return false;

	if (reconstruction.NumViews())
	{
		// Centers the reconstruction based on the absolute deviation of 3D points.
		reconstruction.Normalize();
	}

	// Set up output directories.
	CreateDirectoryIfDoesNotExist(pmvsPath);
	const std::string visualize_dir = pmvsPath + "\\visualize";
	ReCreateDirectory(visualize_dir);
	const std::string txt_dir = pmvsPath + "\\txt";
	ReCreateDirectory(txt_dir);
	const std::string models_dir = pmvsPath + "\\models";
	CreateDirectoryIfDoesNotExist(models_dir);

	std::string FLAGS_images = inputImageDir + "*.jpg";
	const int num_cameras = WriteCamerasToPMVS(reconstruction, pmvsPath, undistort, FLAGS_images);
	WritePMVSOptions(pmvsPath, num_cameras, FLAGS_num_threads);

	const std::string lists_file = pmvsPath + "\\list.txt";
	const std::string bundle_file =
		pmvsPath + "\\bundle.rd.out";

	return	theia::WriteBundlerFiles(reconstruction, lists_file, bundle_file);
}


bool build_reconstruction(std::string& strPathExe, std::string& inputImageDir, std::string& resultString, 
	bool use_gpu, const int FLAGS_num_threads)
{
	std::string FLAGS_images = inputImageDir + "*.jpg";
	std::string FLAGS_matching_working_directory = inputImageDir + "features\\";
	std::string FLAGS_output_matches_file = inputImageDir + "output.matches";

	theia::Reconstruction reconstruction;

	LOG(INFO) << "开始执行稀疏重建：";

	const ReconstructionBuilderOptions options =
		SetReconstructionBuilderOptions(FLAGS_output_matches_file, FLAGS_matching_working_directory, FLAGS_num_threads);

	LOG(INFO) << formatStructure(options);

	ReconstructionBuilder reconstruction_builder(options, strPathExe, use_gpu);
	// If matches are provided, load matches otherwise load images.
	if (FLAGS_images.size() != 0) {
		AddImagesToReconstructionBuilder(&reconstruction_builder, FLAGS_images);
	}
	else {
		LOG(FATAL)
			<< "You must specifiy either images to reconstruct or a match file.";
	}

	std::vector<theia::Reconstruction*> reconstructions;

	if (false == reconstruction_builder.BuildReconstruction(&reconstructions))
	{
		LOG(INFO) << "无法创建重建结果（Could not create a reconstruction）.";
		return false;
	}
	else
	{
		LOG(INFO) << "执行稀疏重建完成！";

		if (reconstructions.size() && reconstructions[0]->NumTracks())
		{
			LOG(INFO) << "稀疏重建三维点的数目为：" << reconstructions[0]->NumTracks();
			reconstruction = *reconstructions[0];
		}
		else
		{
			LOG(INFO) << "稀疏重建三维点的数目为0，重建结束！";
			return -1;
		}

		LOG(INFO) << "开始为点云配置颜色：";
		theia::ColorizeReconstruction(inputImageDir,
			FLAGS_num_threads,
			&reconstruction);
		LOG(INFO) << "为点云配置颜色完成！";

		theia::WriteReconstruction(reconstruction,
			resultString);

		return true;
	}
}

