#ifndef LAUNCH_PMVS2_H
#define LAUNCH_PMVS2_H

#include <string>
#include <glog/logging.h>
#include "utility_common.h"
#include <Windows.h>
#include <Shellapi.h>

int lanch_external_bin(std::string& bin, std::string& parameter, std::string& path, int nShowType = SW_HIDE);

int run_pmvs(std::string& pmvsPath, int threshold_group);

#endif //LAUNCH_PMVS2_H