
#include "stdafx.h"
#include "glCameraNode.h"
#include <math.h>

#define	PI	3.14159
#define KEY_DOWN(vk_code)((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)

cameranode::cameranode()
{
	//	视点位置
	g_eye[0] = -50.0f;	
	g_eye[1] = 200.0f;	
	g_eye[2] = 550.0f;	

	//	视线方向
	g_dir[0] = 0.0f;	
	g_dir[1] = 0.0f;	
	g_dir[2] = -1.0f;	// 默认朝屏幕里面看

	//	头顶法向
	g_up[0] = 0.0f;		
	g_up[1] = -1.0f;		// 头顶向上，不歪
	g_up[2] = 0.0f;		

	g_Angle = -90.0f;
	g_speed = 2.0f;

	m_viewType = VIEW_PERSPECTIVE;

	g_target[0] = 0;
	g_target[1] = 0;
	g_target[2] = 0;

}

void cameranode::look()
{
	if (KEY_DOWN(VK_SHIFT) && (g_speed <= 10000))    g_speed   =g_speed*1.05;//按SHIFT时的加速
	if (KEY_DOWN(VK_CONTROL))  g_speed   =g_speed/1.05;//按CONTROL时的减速
	if (KEY_DOWN(VK_LEFT))     g_Angle-=0.5;//左转
	if (KEY_DOWN(VK_RIGHT))    g_Angle+=0.5;//右转

	if (KEY_DOWN(VK_UP))				//前进
	{
		g_eye[0]+=g_dir[0]*g_speed;
		g_eye[2]+=g_dir[2]*g_speed;
	}
	if (KEY_DOWN(VK_DOWN))			//后退
	{
		g_eye[0]-=g_dir[0]*g_speed;
		g_eye[2]-=g_dir[2]*g_speed; 
	}

	if (KEY_DOWN(VK_HOME))	g_eye[1] +=	g_speed/2;//上升
	if (KEY_DOWN(VK_END))	g_eye[1] -= g_speed/2;//下降

	if (KEY_DOWN(VK_PRIOR) && (g_dir[1] <= 100))	g_dir[1] += 0.02f;//抬头
	if (KEY_DOWN(VK_NEXT) && (g_dir[1] >= -100))	g_dir[1] -= 0.02f;//低头

	switch (m_viewType)
	{
	case VIEW_PERSPECTIVE:

		glTranslatef(0.0f, 0.0f, -0.5 * m_distance);

		gluLookAt(g_eye[0], g_eye[1], g_eye[2],
			g_target[0], g_target[1], g_target[2],
			0.0, -1.0, 0.0);
		break;

	case VIEW_TOP:

		gluLookAt(g_eye[0], g_eye[1], g_eye[2],
			g_target[0], g_target[1], g_target[2],
			0.0, 0.0, -1.0);
		break;

	case VIEW_CAMERA:

		glTranslatef(0.0f, 0.0f, -4.0*0.034 / abs(g_eye[2] / m_distance));

		gluLookAt(g_eye[0], g_eye[1], g_eye[2],
			g_target[0], g_target[1], g_target[2],
			0.0, -1.0, 0.0);

		break;

	case VIEW_FREE:

		g_dir[0] = cos(PI*g_Angle / 180.0f);
		g_dir[2] = sin(PI*g_Angle / 180.0f);

		gluLookAt(
			g_eye[0], g_eye[1], g_eye[2],
			g_eye[0] + g_dir[0], g_eye[1] + g_dir[1], g_eye[2] + g_dir[2],
			g_up[0], g_up[1], g_up[2]);
		break;

	default:
		break;
	}
}

void cameranode::setEye(float* p)
{
	memcpy(g_eye, p, sizeof(float) * 3);
}

void cameranode::setEye(float x, float y, float z)
{
	g_eye[0] = x;
	g_eye[1] = y;
	g_eye[2] = z;
}

float* cameranode::getEye()
{
	return g_eye;
}

void cameranode::setTarget(float* p)
{
	memcpy(g_target, p, sizeof(float) * 3);
}

void cameranode::setTarget(float x, float y, float z)
{
	g_target[0] = x;
	g_target[1] = y;
	g_target[2] = z;
}

float* cameranode::getTarget()
{
	return g_target;
}

void cameranode::setDistance(float d)
{
	m_distance = d;
}

void cameranode::setViewType(int type)
{
	m_viewType = (EnumViewType)type;
}

void cameranode::setSpeed(float s)
{
	g_speed = s;
}
