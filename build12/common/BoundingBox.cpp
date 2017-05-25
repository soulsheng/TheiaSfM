
#include "stdafx.h"
#include "BoundingBox.h"
#include <Windows.h>
#include "gl/GL.H"

void  BoundingBox::calculate(theia::Vector3dVec& allPoints, bool swap_yz, bool y_flip)
{
	setDefault();

	for (theia::Vector3dVec::iterator itr = allPoints.begin(); itr != allPoints.end(); itr++)
	{
		double x = itr->x();
		double y = itr->y();
		double z = itr->z();

		if (swap_yz)
		{
			double tmp = y;
			y = z;
			z = tmp;

			itr->y() = y;
			itr->z() = z;
		}

		if (y_flip)
		{
			y = -y;
			itr->y() = y;
		}

		if (x < minPoint.x())	minPoint.x() = x;
		if (y < minPoint.y())	minPoint.y() = y;
		if (z < minPoint.z())	minPoint.z() = z;

		if (x > maxPoint.x())	maxPoint.x() = x;
		if (y > maxPoint.y())	maxPoint.y() = y;
		if (z > maxPoint.z())	maxPoint.z() = z;
	}

	midPoint = (maxPoint + minPoint) / 2;
	sizePoint = maxPoint - minPoint;

}

BoundingBox::BoundingBox()
{
	setDefault();
}

BoundingBox::~BoundingBox()
{

}


void BoundingBox::setDefault()
{
	int nMaxValue = 1 << 30;
	minPoint.x() = minPoint.y() = minPoint.z() = nMaxValue;
	maxPoint.x() = maxPoint.y() = maxPoint.z() = -nMaxValue;
}

void BoundingBox::DrawBox(float xMin, float yMin, float zMin,
	float xMax, float yMax, float zMax)
{
	GLfloat p1[] = { xMax, yMin, zMin }, p2[] = { xMax, yMax, zMin },
		p3[] = { xMax, yMax, zMax }, p4[] = { xMax, yMin, zMax },
		p5[] = { xMin, yMin, zMax }, p6[] = { xMin, yMax, zMax },
		p7[] = { xMin, yMax, zMin }, p8[] = { xMin, yMin, zMin };


#if 0
		   6_______ 3	
		  /		  /|
		 /		 / |
		7------2   |
		|	   |   |
		|   5--|   4	
		|	   |  /
		|	   | /
		8------1
#endif

		glBegin(GL_LINE_STRIP); //绘制多个四边形

	glVertex3fv(p1);
	glVertex3fv(p2);
	glVertex3fv(p3);
	glVertex3fv(p4);

	glVertex3fv(p5);
	glVertex3fv(p6);
	glVertex3fv(p7);
	glVertex3fv(p8);

	glVertex3fv(p5);
	glVertex3fv(p6);
	glVertex3fv(p3);
	glVertex3fv(p4);

	glVertex3fv(p1);
	glVertex3fv(p2);
	glVertex3fv(p7);
	glVertex3fv(p8);

	glVertex3fv(p2);
	glVertex3fv(p3);
	glVertex3fv(p6);
	glVertex3fv(p7);

	glVertex3fv(p1);
	glVertex3fv(p4);
	glVertex3fv(p5);
	glVertex3fv(p8);

	glEnd();

}

void BoundingBox::DrawBox()
{
	DrawBox( minPoint.x(), minPoint.y(), minPoint.z(),
		maxPoint.x(), maxPoint.y(), maxPoint.z() );
}
