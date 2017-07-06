#ifndef LAUNCH_PMVS2_H
#define LAUNCH_PMVS2_H

#include <string>
#include "utility_common.h"
#include <Shellapi.h>


bool lanch_external_bin(std::string& bin, std::string& parameter, std::string& path, int nShowType = SW_HIDE)
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
	ShExecInfo.nShow = nShowType;
	ShExecInfo.hInstApp = NULL;
	
	if (FALSE == ShellExecuteEx(&ShExecInfo))
	{
		LOG(INFO) << bin << parameter << "无法执行，可能原因：功能模块未找到或者参数不支持！";
		return false;
	}

	long waitStatus = WaitForSingleObject(ShExecInfo.hProcess, INFINITE);

	if (waitStatus)
		printf("failed \n");
	else
		printf("succeed \n");

	return true;
}


void run_pmvs(const char *exeFullPath, std::string& pmvsPath, int threshold_group)
{
	std::string exePath = getPath(std::string(exeFullPath));
	if (false == lanch_external_bin(std::string("cmvs.exe"), pmvsPath, exePath))
		return;

	if (false == lanch_external_bin(std::string("genOption.exe"), pmvsPath, exePath))
		return;

	std::ostringstream parameter;
	parameter << pmvsPath << " option-0000" << " thresholdGroup " << threshold_group;
	lanch_external_bin(std::string("pmvs2.exe"), parameter.str(), exePath);

}

#endif //LAUNCH_PMVS2_H