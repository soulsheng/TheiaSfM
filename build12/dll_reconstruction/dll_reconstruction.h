// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� DLL_RECONSTRUCTION_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// DLL_RECONSTRUCTION_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef DLL_RECONSTRUCTION_EXPORTS
#define DLL_RECONSTRUCTION_API __declspec(dllexport)
#else
#define DLL_RECONSTRUCTION_API __declspec(dllimport)
#endif

// �����Ǵ� dll_reconstruction.dll ������
class DLL_RECONSTRUCTION_API Cdll_reconstruction {
public:
	Cdll_reconstruction(void);
	// TODO:  �ڴ�������ķ�����
};

extern DLL_RECONSTRUCTION_API int ndll_reconstruction;

DLL_RECONSTRUCTION_API int fndll_reconstruction(void);
