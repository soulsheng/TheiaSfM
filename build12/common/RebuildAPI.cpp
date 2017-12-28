
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

//DEFINE_bool(use_gpu, true, "use gpu of sift and other modual.");

// Multithreading.
DEFINE_int32(num_threads, 4,
	"Number of threads to use for feature extraction and matching.");

#define FLAG_FILE_NAME	"build_reconstruction_flags.txt"

//#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

#if 1
String FLAGS_ply_file;
#else
DEFINE_string(pmvs_working_directory, "",
	"A directory to store the necessary pmvs files.");
DEFINE_string(ply_file, "option-0000.ply", "Output PLY file.");
#endif

#include <fstream>  // NOLINT

extern "C" DLL_RECONSTRUCTION_API int	kernelReBuildDense(char* pInputImageDir, char* filename_sparse, char* filename_dense, bool isLogInitialized)
{
	if (!FLAGS_build)
		return -1;

	std::string inputImageDir(pInputImageDir);
	std::string pmvsPath = inputImageDir + "pmvs\\";

	std::string exePath = getEXEDLLFullPath();

	if (!isLogInitialized)
		SetLog(exePath);

#if 1
	if (!export_to_pmvs(pmvsPath, inputImageDir, std::string(filename_sparse), FLAGS_num_threads, FLAGS_undistort))
		return -2;
#endif


	LOG(INFO) << "开始执行稠密重建：";
#if 1
	run_pmvs(pmvsPath, FLAGS_threshold_group);
#endif
	LOG(INFO) << "执行稠密重建完成！";

	std::string str_ply_file = pmvsPath + "models\\option-0000.ply";
	strcpy(filename_dense, str_ply_file.c_str());

	return 0;
}


void SetLog(std::string &exePath)
{
	google::InitGoogleLogging(exePath.c_str());


	google::SetLogDestination(google::GLOG_INFO, exePath.c_str());
	google::SetLogDestination(google::GLOG_ERROR, "");
	google::SetLogDestination(google::GLOG_WARNING, "");
	google::SetLogFilenameExtension(".log");
}

extern "C" DLL_RECONSTRUCTION_API int kernelReBuildSparse(char* pInputImageDir, char* result_filename, bool use_gpu)
{
	std::string exePath = getEXEDLLFullPath();
	std::string inputImageDir(pInputImageDir);
	std::string FLAGS_output_reconstruction;
	
	std::string resultString(result_filename);
	if ( !resultString.empty() )
		FLAGS_output_reconstruction = inputImageDir + resultString;
	else
	{
		FLAGS_output_reconstruction = inputImageDir + "result";
		strcpy(result_filename, FLAGS_output_reconstruction.c_str());
	}



	//THEIA_GFLAGS_NAMESPACE::ParseCommandLineFlags(&argc, &argv, true);
	SetLog(exePath);

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
#if 0
	LOG(INFO) << "create log";
	std::ostringstream os;
	os << exePath << std::endl << inputImageDir << std::endl;
	strcpy(pResultString, os.str().c_str());
	return -10;
#endif


	build_reconstruction(strPathExe, inputImageDir, FLAGS_output_reconstruction, use_gpu, FLAGS_num_threads);

	return 0;
}

extern "C" DLL_RECONSTRUCTION_API std::string get_Path(std::string& strFullPath)
{
	return getPath(strFullPath);
}

extern "C" DLL_RECONSTRUCTION_API bool lanch_external(std::string& bin, std::string& parameter, std::string& path, int nShowType)
{
	return lanch_external_bin(bin, parameter, path, nShowType);
}

extern "C" DLL_RECONSTRUCTION_API std::string get_EXEDLLPath()
{
	return getEXEDLLFullPath();
}
