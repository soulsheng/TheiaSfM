
#include "RebuildAPI.h"


#include <Eigen/Core>
#include <glog/logging.h>
//#include <gflags/gflags.h>
#include <theia/theia.h>
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



#include <fstream>  // NOLINT

void rand_num_views_for_track(std::vector<int>& num_views_for_track, int size)
{
	num_views_for_track.reserve(size);
	for (int i = 0; i < size; i++)
		num_views_for_track.emplace_back(rand() % min_num_views_for_track);
}

void	kernelReBuildDense(std::string& pmvsPath, std::string& ply_file)
{
	if (!FLAGS_build)
		return;

#if 1
	if (!export_to_pmvs(pmvsPath, FLAGS_undistort))
		return;
#endif


	LOG(INFO) << "开始执行稠密重建：";
#if 1
	run_pmvs(strPathExe.c_str(), pmvsPath, FLAGS_threshold_group);
#endif
	LOG(INFO) << "执行稠密重建完成！";

}


void kernelReBuildSparse(std::string &exePath, std::string& inputImageDir)
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
	

	//############### Logging Options ###############
	//  # Logging verbosity.
	FLAGS_logtostderr = false;
	// # Increase this number to get more verbose logging.
	FLAGS_v = 1;

	min_num_views_for_track = FLAGS_fps * FLAGS_length;

	ReCreateDirectory(FLAGS_matching_working_directory);


	build_reconstruction(strPathExe, inputImageDir);


}

void	viewDenseResult(std::string& ply_file)
{
	world_points.clear();
	point_normals.clear();
	point_colors.clear();

	clock_t tBegin = clock();
	//std::cout << "ReadPlyFile begin: " << tBegin << std::endl;
	if (!theia::ReadPlyFile(ply_file, world_points, point_normals, point_colors))
		printf("can not open ply file!\n");
	//std::cout << "ReadPlyFile cost " << (clock() - tBegin) / 1000 << " seconds" << std::endl;

	rand_num_views_for_track(num_views_for_track, world_points.size());

	box.calculate(world_points, FLAGS_swap_yz);

	FLAGS_view_type = VIEW_PERSPECTIVE;

	setDefaultCameraProperty();

	LOG(INFO) << "稠密重建三维点的数目为：" << world_points.size();
	LOG(INFO) << "输出稠密重建结果！";
}

void render3DResult(std::string &exePath, std::string& ply_file, std::string outputImageDir,
	std::string& pmvsPath)
{
	viewDenseResult(ply_file);

	gl_draw_points(1, (char*)exePath.c_str(), outputImageDir, pmvsPath, ply_file);
}
