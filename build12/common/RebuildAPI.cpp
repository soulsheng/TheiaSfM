
#include "RebuildAPI.h"


#include <Eigen/Core>
#include <glog/logging.h>
#include <gflags/gflags.h>
#include <theia/theia.h>
#include <vector>

#include "build_common.h"



#include "utility_theia.h"

#include "LaunchPMVS2.h"

typedef std::string	String;


DEFINE_bool(undistort, false, "bool on/off to undistort image. eg:0 ");

DEFINE_bool(build, true, "bool on/off to build. eg:0 ");

DEFINE_bool(build_sparse, true, "bool on/off to build. eg:0 ");

DEFINE_int32(threshold_group, 35, "threshodGroup to filter group of outlier points.");

DEFINE_bool(use_gpu, true, "use gpu of sift and other modual.");

// Multithreading.
DEFINE_int32(num_threads, 4,
	"Number of threads to use for feature extraction and matching.");

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

void	kernelReBuildDense(std::string &exePath, std::string& pmvsPath, std::string& ply_file, std::string& inputImageDir)
{
	if (!FLAGS_build)
		return;

#if 1
	if (!export_to_pmvs(pmvsPath, FLAGS_undistort, inputImageDir, FLAGS_num_threads))
		return;
#endif


	LOG(INFO) << "��ʼִ�г����ؽ���";
#if 1
	run_pmvs(exePath.c_str(), pmvsPath, FLAGS_threshold_group);
#endif
	LOG(INFO) << "ִ�г����ؽ���ɣ�";

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

	std::string strPathExe = exePath.c_str();
	strPathExe = strPathExe.substr(0, strPathExe.find_last_of("\\") + 1);

	//google::ReadFromFlagsFile(FLAG_FILE_NAME, strPathExe.c_str(), false);
	//std::cout << "exe path..." << strPathExe << std::endl;
	std::string FLAGS_output_matches_file = inputImageDir + "output.matches";
	std::string FLAGS_matching_working_directory = inputImageDir + "features\\";
	

	//############### Logging Options ###############
	//  # Logging verbosity.
	FLAGS_logtostderr = false;
	// # Increase this number to get more verbose logging.
	FLAGS_v = 1;


	ReCreateDirectory(FLAGS_matching_working_directory);


	build_reconstruction(strPathExe, inputImageDir, FLAGS_use_gpu, FLAGS_num_threads);


}
