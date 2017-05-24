#ifndef LAUNCH_PMVS2_H
#define LAUNCH_PMVS2_H

#include <string>

#ifndef String
typedef std::string String;
#endif

void lanch_external_bin(String& bin, String& parameter, String& path)
{
	String zipParameter = String("a -m0 -inul -idp -sfxDefault.SFX -ibck -iiconVRGIS.ico -zsescript ");

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

String getPath(String& strFullPath)
{
	int indexEnd = strFullPath.find_last_of('\\');
	return strFullPath.substr(0, indexEnd + 1);
}

void run_pmvs(const char *exeFullPath, String& pmvsPath, int threshold_group)
{
	String exePath = getPath(String(exeFullPath));
	lanch_external_bin(String("cmvs.exe"), pmvsPath, exePath);

	lanch_external_bin(String("genOption.exe"), pmvsPath, exePath);

	std::ostringstream parameter;
	parameter << pmvsPath << " option-0000" << " thresholdGroup " << threshold_group;
	lanch_external_bin(String("pmvs2.exe"), parameter.str(), exePath);

}

#endif //LAUNCH_PMVS2_H