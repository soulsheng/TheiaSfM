
#include "LaunchPMVS2.h"
#include "utility_common.h"

int lanch_external_bin(std::string& bin, std::string& parameter, std::string& path, int nShowType)
{
	int nRetCode = 0;
	std::string exeFullName = path + bin;
	if (!fileExist(exeFullName))
	{
		nRetCode = -45;

		LOG(INFO) << "异常返回！异常代码：" << nRetCode << std::endl
			<< "异常描述：稠密重建失败-缺少稠密重建模块，可能原因：稠密重建模块文件不存在，"
			<< "建议措施：检查exe目录下是否存在cmvs.exe/genOption.exe/pmvs2.exe: "
			<< exeFullName;

		return nRetCode;
	}


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
		nRetCode = -46;

		LOG(INFO) << "异常返回！异常代码：" << nRetCode << std::endl
			<< "异常描述：稠密重建失败-稠密重建模块执行失败，可能原因：参数不支持，"
			<< "建议措施：检查参数是否配置正确"
			<< path << bin << parameter;

		return nRetCode;;
	}

	long waitStatus = WaitForSingleObject(ShExecInfo.hProcess, INFINITE);

	if (waitStatus)
		printf("failed \n");
	else
		printf("succeed \n");

	return nRetCode;
}


int run_pmvs(std::string& pmvsPath, int threshold_group)
{
	std::string exePath = getEXEDLLPath();

	int nRetCode = 0;
	
	nRetCode = lanch_external_bin(std::string("cmvs.exe"), pmvsPath, exePath);

	if (nRetCode != 0)
		return nRetCode;

	nRetCode = lanch_external_bin(std::string("genOption.exe"), pmvsPath, exePath);

	if (nRetCode != 0)
		return nRetCode;

	std::ostringstream parameter;
	parameter << pmvsPath << " option-0000" << " thresholdGroup " << threshold_group;
	
	return lanch_external_bin(std::string("pmvs2.exe"), parameter.str(), exePath);

}
