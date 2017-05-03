
#ifndef CAMERANODE_H
#define CAMERANODE_H

struct cameranode
{
	float	g_eye[3];		//	�ӵ�λ��
	float	g_dir[3];		//	���߷���
	float	g_up[3];		//	ͷ������
	cameranode();
	void	look();
	void	setEye(float* p);
	void	setEye(float x, float y, float z);
	float*	getEye();
	void	setSpeed(float s);

	float		g_Angle;		//����ת
	float		g_speed;		//�ٶ�
};


#endif // CAMERANODE_H