
#include "stdafx.h"
#include "glCameraNode.h"
#include <math.h>

#define	PI	3.14159
#define KEY_DOWN(vk_code)((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)

cameranode::cameranode()
{
	//	�ӵ�λ��
	g_eye[0] = -50.0f;	
	g_eye[1] = 200.0f;	
	g_eye[2] = 550.0f;	

	//	���߷���
	g_dir[0] = 0.0f;	
	g_dir[1] = 0.0f;	
	g_dir[2] = -1.0f;	// Ĭ�ϳ���Ļ���濴

	//	ͷ������
	g_up[0] = 0.0f;		
	g_up[1] = -1.0f;		// ͷ�����ϣ�����
	g_up[2] = 0.0f;		

	g_Angle = -90.0f;
	g_speed = 2.0f;
}

void cameranode::look()
{
	if (KEY_DOWN(VK_SHIFT) && (g_speed <= 10000))    g_speed   =g_speed*1.05;//��SHIFTʱ�ļ���
	if (KEY_DOWN(VK_CONTROL))  g_speed   =g_speed/1.05;//��CONTROLʱ�ļ���
	if (KEY_DOWN(VK_LEFT))     g_Angle-=0.5;//��ת
	if (KEY_DOWN(VK_RIGHT))    g_Angle+=0.5;//��ת

	if (KEY_DOWN(VK_UP))				//ǰ��
	{
		g_eye[0]+=g_dir[0]*g_speed;
		g_eye[2]+=g_dir[2]*g_speed;
	}
	if (KEY_DOWN(VK_DOWN))			//����
	{
		g_eye[0]-=g_dir[0]*g_speed;
		g_eye[2]-=g_dir[2]*g_speed; 
	}

	if (KEY_DOWN(VK_HOME))	g_eye[1] +=	g_speed/2;//����
	if (KEY_DOWN(VK_END))	g_eye[1] -= g_speed/2;//�½�

	if (KEY_DOWN(VK_PRIOR) && (g_dir[1] <= 100))	g_dir[1] += 0.02f;//̧ͷ
	if (KEY_DOWN(VK_NEXT) && (g_dir[1] >= -100))	g_dir[1] -= 0.02f;//��ͷ


	g_dir[0] = cos(PI*g_Angle/180.0f);
	g_dir[2] = sin(PI*g_Angle/180.0f);

	gluLookAt(
		g_eye[0],	g_eye[1],	g_eye[2],
		g_eye[0] + g_dir[0],	g_eye[1] + g_dir[1],	g_eye[2] + g_dir[2],
		g_up[0],	g_up[1],	g_up[2]);

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

void cameranode::setSpeed(float s)
{
	g_speed = s;
}
