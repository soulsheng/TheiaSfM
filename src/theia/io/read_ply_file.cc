// Copyright (C) 2015 The Regents of the University of California (Regents).
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//
//     * Neither the name of The Regents or University of California nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Please contact the author of this library if you have any questions.
// Author: Chris Sweeney (cmsweeney@cs.ucsb.edu)

#include "theia/io/read_ply_file.h"

#include <glog/logging.h>
#include <fstream>  // NOLINT
#include <string>
#include <vector>

#include "theia/sfm/reconstruction.h"

namespace theia {

	enum PointProperty
	{
		POSITION,
		NORMAL,
		COLOR,
		PROPERTY_COUNT
	};

void getPointCountFromPlyHeader(std::ifstream &ply_reader, int& nPointCount, bool* bProperty)
{
	char	sBuffer[128];

	// ignore first 2 lines
	for (int i = 0; i < 2; i++)
		ply_reader.getline(sBuffer, sizeof(sBuffer));

	// get point count from the third line
	ply_reader >> sBuffer;
	ply_reader >> sBuffer;
	ply_reader >> nPointCount;

	// ignore 10 lines more
	std::string sPointProperty ;
	for (int i = 0; i < 10; i++)
	{
		ply_reader.getline(sBuffer, sizeof(sBuffer));

		if ( std::string("end_header") == std::string(sBuffer) )
			break;

		sPointProperty += sBuffer;
		sPointProperty += " ";
	}

	if ( -1 != sPointProperty.find("x"))
		bProperty[POSITION] = true;

	if (-1 != sPointProperty.find("nx"))
		bProperty[NORMAL] = true;

	if (-1 != sPointProperty.find("red"))
		bProperty[COLOR] = true;

}

// Reads a PLY file - a common format of MeshLab.
bool ReadPlyFile(const std::string& ply_file,
				  Vector3dVec& points_to_read,
				  Vector3dVec& normals_to_read,
				  Vector3iVec& colors_to_read)
{
  CHECK_GT(ply_file.length(), 0);

  // Return false if the file cannot be opened for reading.
  std::ifstream ply_reader(ply_file, std::ifstream::in);
  if (!ply_reader.is_open()) {
    LOG(ERROR) << "Could not open the file: " << ply_file
               << " for reading a PLY file.";
    return false;
  }


  // Extract points that we will read to the PLY file.
  int nPointCount = 0;
  bool	bProperty[3] = {false, false, false};	// position, normal, color
  getPointCountFromPlyHeader(ply_reader, nPointCount, bProperty);

  points_to_read.reserve(nPointCount);
  normals_to_read.reserve(nPointCount);
  colors_to_read.reserve(nPointCount);

  float fPoint[3];
  float fNormal[3];
  int cColor[3];
  for (int i = 0; i < nPointCount; i++) {

	  if ( bProperty[POSITION] )
	  {
		ply_reader >> fPoint[0] >> fPoint[1] >> fPoint[2];
		points_to_read.emplace_back( Eigen::Vector3d(fPoint[0], fPoint[1], fPoint[2]) );
		points_to_read[i].transpose();
	  }

	  if (bProperty[NORMAL])
	  {
		  ply_reader >> fNormal[0] >> fNormal[1] >> fNormal[2];
		  normals_to_read.emplace_back(Eigen::Vector3d(fNormal[0], fNormal[1], fNormal[2]) );
		  normals_to_read[i].transpose();
	  }

	  if (bProperty[COLOR])
	  {
		  ply_reader >> cColor[0] >> cColor[1] >> cColor[2];
		  colors_to_read.emplace_back(Eigen::Vector3i(cColor[0], cColor[1], cColor[2]) );
		  colors_to_read[i].transpose();
	  }

  }

  return true;
}

bool WritePlyFile(const std::string& ply_file, Vector3dVec& points_to_write, Vector3dVec& normals_to_write, Vector3iVec& colors_to_write)
{

	// Return false if the file cannot be opened for writing.
	std::ofstream ply_writer(ply_file, std::ofstream::out);
	if (!ply_writer.is_open()) {
		LOG(ERROR) << "Could not open the file: " << ply_file
			<< " for writing a PLY file.";
		return false;
	}

	ply_writer << "ply"
		<< '\n' << "format ascii 1.0"
		<< '\n' << "element vertex " << points_to_write.size()
		<< '\n' << "property float x"
		<< '\n' << "property float y"
		<< '\n' << "property float z"
		<< '\n' << "property uchar red"
		<< '\n' << "property uchar green"
		<< '\n' << "property uchar blue"
		<< '\n' << "end_header" << std::endl;


	for (int i = 0; i < points_to_write.size(); i++) {
		if (points_to_write.size())
			ply_writer << points_to_write[i].transpose() << " ";

		if (normals_to_write.size())
			ply_writer << normals_to_write[i].transpose() << "\n";

		if (colors_to_write.size())
			ply_writer << colors_to_write[i].transpose() << "\n";
	}
	
	return true;
}

}  // namespace theia
