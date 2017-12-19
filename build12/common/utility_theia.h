
#ifndef UTILITY_THEIA_H
#define UTILITY_THEIA_H

#include <glog/logging.h>
#include <theia/theia.h>

void CreateDirectoryIfDoesNotExist(const std::string& directory);

void ReCreateDirectory(const std::string& directory);

int WriteCamerasToPMVS(const theia::Reconstruction& reconstruction, std::string& pmvsPath, 
	bool undistort, std::string& FLAGS_images);

void WritePMVSOptions(const std::string& working_dir,
	const int num_images);

bool export_to_pmvs(std::string& pmvsPath, bool undistort, std::string& inputImageDir, 
	const int FLAGS_num_threads);

bool build_reconstruction(std::string& strPathExe, std::string& inputImageDir, 
	bool use_gpu, const int FLAGS_num_threads);


#endif // UTILITY_THEIA_H
