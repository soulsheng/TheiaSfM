
#pragma once

#include "theia/sfm/types.h"
#include "common_type.h"

class BoundingBox
{
public:
	void	calculate(theia::Vector3dVec& allPoints, bool swap_yz = false, bool y_flip = false);
	
	void	DrawBox(float xMin, float yMin, float zMin,
		float xMax, float yMax, float zMax);

	void	DrawBox();

	Eigen::Vector3f& getMinPoint()	{ return minPoint; }
	Eigen::Vector3f& getMaxPoint()	{ return maxPoint; }
	Eigen::Vector3f& getMidPoint()	{ return midPoint; }
	Eigen::Vector3f& getSizePoint()	{ return sizePoint; }

	BoundingBox();
	~BoundingBox();

protected:
	void	setDefault();

protected:
	Eigen::Vector3f minPoint, maxPoint;
	Eigen::Vector3f midPoint, sizePoint;

};
