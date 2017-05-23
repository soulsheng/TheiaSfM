
#ifndef CAMERANODE_H
#define CAMERANODE_H

#include "common_type.h"

struct cameranode
{
	float	g_eye[3];		//	�ӵ�λ��
	float	g_dir[3];		//	���߷���
	float	g_up[3];		//	ͷ������
	float	g_target[3];	//	Ŀ�꽹��
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

	float		g_Angle;		//����ת
	float		g_speed;		//�ٶ�
	EnumViewType	m_viewType;	//�ۿ����� 
	float		m_distance;
};


#endif // CAMERANODE_H