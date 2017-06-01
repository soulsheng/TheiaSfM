#ifndef LAUNCH_PMVS2_H
#define LAUNCH_PMVS2_H

#include <string>
#include "utility_common.h"
#include <Shellapi.h>


void lanch_external_bin(std::string& bin, std::string& parameter, std::string& path)
{
	std::string zipParameter = std::string("a -m0 -inul -idp -sfxDefault.SFX -ibck -iiconVRGIS.ico -zsescript ");

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


void run_pmvs(const char *exeFullPath, std::string& pmvsPath, int threshold_group)
{
	std::string exePath = getPath(std::string(exeFullPath));
	lanch_external_bin(std::string("cmvs.exe"), pmvsPath, exePath);

	lanch_external_bin(std::string("genOption.exe"), pmvsPath, exePath);

	std::ostringstream parameter;
	parameter << pmvsPath << " option-0000" << " thresholdGroup " << threshold_group;
	lanch_external_bin(std::string("pmvs2.exe"), parameter.str(), exePath);

}

#endif //LAUNCH_PMVS2_H