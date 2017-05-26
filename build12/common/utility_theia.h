
#ifndef UTILITY_THEIA_H
#define UTILITY_THEIA_H

#include <theia/theia.h>

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

int WriteCamerasToPMVS(const theia::Reconstruction& reconstruction, std::string& pmvsPath, bool undistort) {
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
		theia::Camera undistorted_camera;
		CHECK(theia::UndistortCamera(distorted_camera, &undistorted_camera));

		theia::FloatImage distorted_image(image_files[i]);
		theia::FloatImage undistorted_image;
		if (undistort)
			CHECK(theia::UndistortImage(distorted_camera,
			distorted_image,
			undistorted_camera,
			&undistorted_image));
		else
			undistorted_image = distorted_image;

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

bool export_to_pmvs(theia::Reconstruction& reconstruction, std::string& pmvsPath, bool undistort)
{
	// Set up output directories.
	CreateDirectoryIfDoesNotExist(pmvsPath);
	const std::string visualize_dir = pmvsPath + "/visualize";
	ReCreateDirectory(visualize_dir);
	const std::string txt_dir = pmvsPath + "/txt";
	ReCreateDirectory(txt_dir);
	const std::string models_dir = pmvsPath + "/models";
	CreateDirectoryIfDoesNotExist(models_dir);

	const int num_cameras = WriteCamerasToPMVS(reconstruction, pmvsPath, undistort);
	WritePMVSOptions(pmvsPath, num_cameras);

	const std::string lists_file = pmvsPath + "/list.txt";
	const std::string bundle_file =
		pmvsPath + "/bundle.rd.out";

	return	theia::WriteBundlerFiles(reconstruction, lists_file, bundle_file);		
}

std::string formatStructure(theia::ReconstructionBuilderOptions options)
{
	std::ostringstream os;
	os << std::endl << std::endl << "配置参数如下：" << std::endl;
	os << "num_threads = " << options.num_threads << std::endl ;
	os << "output_matches_file = " << options.output_matches_file << std::endl ;

	os << "descriptor_type = " << FLAGS_descriptor << std::endl;

	os << "feature_density = " << FLAGS_feature_density << std::endl;
	os << "match_out_of_core = " << options.matching_options.match_out_of_core << std::endl ;
	os << "keypoints_and_descriptors_output_dir = " << options.matching_options.keypoints_and_descriptors_output_dir << std::endl ;
	os << "cache_capacity = " << options.matching_options.cache_capacity << std::endl ;

	os << "matching_strategy = " << FLAGS_matching_strategy << std::endl;

	os << "lowes_ratio = " << options.matching_options.lowes_ratio << std::endl ;
	os << "keep_only_symmetric_matches = " << options.matching_options.keep_only_symmetric_matches << std::endl ;
	os << "min_num_inlier_matches = " << options.min_num_inlier_matches << std::endl ;
	os << "perform_geometric_verification = " << options.matching_options.perform_geometric_verification << std::endl ;
	os << "FLAGS_max_sampson_error_for_verified_match = " <<  FLAGS_max_sampson_error_for_verified_match << std::endl ;
	os << "FLAGS_bundle_adjust_two_view_geometry = " << FLAGS_bundle_adjust_two_view_geometry << std::endl ;
	os << "FLAGS_triangulation_reprojection_error_pixels = " << FLAGS_triangulation_reprojection_error_pixels << std::endl ;
	os << "FLAGS_min_triangulation_angle_degrees = " << FLAGS_min_triangulation_angle_degrees << std::endl ;
	os << "FLAGS_max_reprojection_error_pixels = " << FLAGS_max_reprojection_error_pixels << std::endl ;

	os << "FLAGS_min_track_length = " << FLAGS_min_track_length << std::endl ;
	os << "FLAGS_max_track_length = " << FLAGS_max_track_length << std::endl ;

	// Reconstruction Estimator Options.
	os << "FLAGS_min_num_inliers_for_valid_match = " << FLAGS_min_num_inliers_for_valid_match << std::endl ;
	os << "FLAGS_intrinsics_to_optimize = " << FLAGS_intrinsics_to_optimize << std::endl ;
	os << "FLAGS_reconstruct_largest_connected_component = " << FLAGS_reconstruct_largest_connected_component << std::endl ;
	os << "FLAGS_only_calibrated_views = " << FLAGS_only_calibrated_views << std::endl ;
	os << "FLAGS_max_reprojection_error_pixels = " << FLAGS_max_reprojection_error_pixels << std::endl ;

	// Which type of SfM pipeline to use (e.g., incremental, global, etc.) << std::endl ;
	os << "FLAGS_reconstruction_estimator = " << FLAGS_reconstruction_estimator << std::endl ;

	// Global SfM Options.
	os << "FLAGS_global_rotation_estimator = " << FLAGS_global_rotation_estimator << std::endl ;
	os << "FLAGS_global_position_estimator = " << FLAGS_global_position_estimator << std::endl ;
	os << "FLAGS_num_retriangulation_iterations = " << FLAGS_num_retriangulation_iterations << std::endl ;
	os << "FLAGS_refine_relative_translations_after_rotation_estimation = " <<	FLAGS_refine_relative_translations_after_rotation_estimation << std::endl ;
	os << "FLAGS_extract_maximal_rigid_subgraph = " << FLAGS_extract_maximal_rigid_subgraph << std::endl ;
	os << "FLAGS_filter_relative_translations_with_1dsfm = " << FLAGS_filter_relative_translations_with_1dsfm << std::endl ;
	os << "FLAGS_post_rotation_filtering_degrees = " << FLAGS_post_rotation_filtering_degrees << std::endl ;
	os << "FLAGS_position_estimation_min_num_tracks_per_view = " << FLAGS_position_estimation_min_num_tracks_per_view << std::endl ;
	os << "FLAGS_refine_camera_positions_and_points_after_position_estimation = " << FLAGS_refine_camera_positions_and_points_after_position_estimation << std::endl ;

	// Triangulation options (used by all SfM pipelines).
	os << "FLAGS_min_triangulation_angle_degrees = " << FLAGS_min_triangulation_angle_degrees << std::endl ;
	os << "FLAGS_triangulation_reprojection_error_pixels = " << FLAGS_triangulation_reprojection_error_pixels << std::endl ;
	os << "FLAGS_bundle_adjust_tracks = " << FLAGS_bundle_adjust_tracks << std::endl ;

	// Bundle adjustment options (used by all SfM pipelines).
	os << "FLAGS_bundle_adjustment_robust_loss_function = " << FLAGS_bundle_adjustment_robust_loss_function << std::endl ;
	os << "FLAGS_bundle_adjustment_robust_loss_width = " << FLAGS_bundle_adjustment_robust_loss_width << std::endl ;
	os << std::endl;

	return std::string(os.str());
}


bool build_reconstruction(Reconstruction* &reconstruction, std::string& strPathExe)
{
	LOG(INFO) << "开始执行稀疏重建：";
	
	const ReconstructionBuilderOptions options =
		SetReconstructionBuilderOptions();

	LOG(INFO) << formatStructure(options);

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

	std::vector<Reconstruction*> reconstructions;

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
			reconstruction = reconstructions[0];
		}
		else
		{
			LOG(INFO) << "稀疏重建三维点的数目为0，重建结束！";
			return -1;
		}

		LOG(INFO) << "开始为点云配置颜色：";
		theia::ColorizeReconstruction(FLAGS_input_images,
			FLAGS_num_threads,
			reconstruction);
		LOG(INFO) << "为点云配置颜色完成！";

		theia::WriteReconstruction(*reconstruction,
			FLAGS_output_reconstruction);

		return true;
	}
}

#endif // UTILITY_THEIA_H
