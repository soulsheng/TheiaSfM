
#ifndef CAMERANODE_H
#define CAMERANODE_H

#include "common_type.h"

struct cameranode
{
	float	g_eye[3];		//	视点位置
	float	g_dir[3];		//	视线方向
	float	g_up[3];		//	头顶法向
	float	g_target[3];	//	目标焦点
	cameranode();
	void	look();
	void	setEye(float* p);
	void	setEye(float x, float y, float z);
	float*	getEye();
	void	setSpeed(float s);

	void	setTarget(float* p);
	void	setTarget(float x, float y, float z);
	float*	getTarget();

	void	setDistance(float d);
	void	setViewType(int type);

	float		g_Angle;		//左右转
	float		g_speed;		//速度
	EnumViewType	m_viewType;	//观看类型 
	float		m_distance;
};


#endif // CAMERANODE_H