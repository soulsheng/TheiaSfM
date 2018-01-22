
#include "LaunchPMVS2.h"
#include "utility_common.h"
#include <theia/io/read_ply_file.h>

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
	
	nRetCode = lanch_external_bin(std::string("pmvs2.exe"), parameter.str(), exePath);


	theia::Vector3dVec world_points;
	theia::Vector3fVec point_normals;
	theia::Vector3fVec point_colors;

	std::string str_ply_file = pmvsPath + "models\\option-0000.ply";
	if (!theia::ReadPlyFile(str_ply_file, world_points, point_normals, point_colors))
		printf("can not open ply file!\n");

	LOG(INFO) << "稠密重建三维点的数目为：" << world_points.size();

	if (0 == world_points.size())
	{
		nRetCode = -47;

		LOG(INFO) << "异常返回！异常代码：" << nRetCode << std::endl
			<< "异常描述：稀疏点全部滤除，剩余0个三维点；可能原因：图片数目过少；"
			<< "建议措施：增加图片数目";
	}

	return nRetCode;

}
