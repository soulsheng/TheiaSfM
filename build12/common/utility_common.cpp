
#include "stdafx.h"
#include "utility_common.h"
#include <string.h>
#include <stdlib.h>  

#include <opencv2/core/types_c.h>  // include it before #include <OpenImageIO/imagebufalgo.h>

#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>
#include <stlplus3/file_system.hpp>
#include "theia/util/filesystem.h"

std::string getPath(std::string& strFullPath)
{
	int indexEnd = strFullPath.find_last_of('\\');
	return strFullPath.substr(0, indexEnd + 1);
}

void getPath(const char* strFullPath, char* pPath)
{
	std::string strPath(strFullPath);
	strPath = getPath(strPath);
	strcpy_s(pPath, sizeof(pPath), strPath.c_str());
	pPath[strPath.size()] = '\0';
}


void resizeImageFiles(std::vector<std::string>& image_files, int nResize, bool bForceResize )
{
	if (image_files.empty())
		return;

	if (image_files.size() > 60)
		nResize = 1280;

	float scale = 1.0f;


	OpenImageIO::ImageBuf image_;
	image_.reset(image_files[0]);
	image_.read(0, 0, true, OpenImageIO::TypeDesc::UCHAR);
	int width_old = image_.spec().width;

	if (!bForceResize && width_old < nResize
		|| width_old == nResize)
		return;

	scale = nResize * 1.0 / width_old;

	int i = 0;
	std::vector<std::string> image_files_new;
	for (const std::string& image_file : image_files)
	{
		OpenImageIO::ImageBuf image_;
		image_.reset(image_file);
		image_.read(0, 0, true, OpenImageIO::TypeDesc::UCHAR);

		OpenImageIO::ROI roi(0, scale *image_.spec().width, 0, scale * image_.spec().height,
			0, 1, 0, image_.nchannels());

		OpenImageIO::ImageBuf dst;
		OpenImageIO::ImageBufAlgo::resize(dst, image_, nullptr, roi);

		std::string image_path;
		theia::GetDirectoryFromFilepath(image_file, &image_path);

		std::string ext = image_file.substr(image_file.find_last_of("."));

		std::ostringstream os;
		os << image_path << "\\" << i++ << ext;

		dst.write(os.str());
		image_files_new.push_back(os.str());
	}

	for (const std::string& image_file : image_files)
	{
		bool breturn = stlplus::file_delete(image_file);
		if (!breturn)
		{
			std::cout << "failed to delete " << image_file;
		}
	}


	image_files = image_files_new;
}

std::string getEXEDLLPath()
{
	std::string path_data = getEXEDLLFullPath();
	return path_data.substr(0, path_data.find_last_of("\\") + 1);
}

std::string getEXEDLLFullPath()
{
	char szPath[MAX_PATH];
	GetModuleFileNameA(NULL, szPath, MAX_PATH);

	return std::string(szPath);
}

bool fileExist(std::string& filename)
{
	return theia::FileExists(filename);
}
