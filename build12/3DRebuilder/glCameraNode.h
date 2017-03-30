
#ifndef CAMERANODE_H
#define CAMERANODE_H

struct cameranode
{
	float	g_eye[3];		//	视点位置
	float	g_dir[3];		//	视线方向
	float	g_up[3];		//	头顶法向
	cameranode();
	void	look();
	void	setEye(float* p);
	void	setEye(float x, float y, float z);
	float*	getEye();

	float		g_Angle;		//左右转
	float		g_speed;		//速度
};


#endif // CAMERANODE_H