
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

		LOG(INFO) << "�쳣���أ��쳣���룺" << nRetCode << std::endl
			<< "�쳣�����������ؽ�ʧ��-ȱ�ٳ����ؽ�ģ�飬����ԭ�򣺳����ؽ�ģ���ļ������ڣ�"
			<< "�����ʩ�����exeĿ¼���Ƿ����cmvs.exe/genOption.exe/pmvs2.exe: "
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

		LOG(INFO) << "�쳣���أ��쳣���룺" << nRetCode << std::endl
			<< "�쳣�����������ؽ�ʧ��-�����ؽ�ģ��ִ��ʧ�ܣ�����ԭ�򣺲�����֧�֣�"
			<< "�����ʩ���������Ƿ�������ȷ"
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

	LOG(INFO) << "�����ؽ���ά�����ĿΪ��" << world_points.size();

	if (0 == world_points.size())
	{
		nRetCode = -47;

		LOG(INFO) << "�쳣���أ��쳣���룺" << nRetCode << std::endl
			<< "�쳣������ϡ���ȫ���˳���ʣ��0����ά�㣻����ԭ��ͼƬ��Ŀ���٣�"
			<< "�����ʩ������ͼƬ��Ŀ";
	}

	return nRetCode;

}
