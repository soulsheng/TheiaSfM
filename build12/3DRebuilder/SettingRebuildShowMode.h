#pragma once


// CSettingRebuildShowMode �Ի���

class CSettingRebuildShowMode : public CDialogEx
{
	DECLARE_DYNAMIC(CSettingRebuildShowMode)

public:
	CSettingRebuildShowMode(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CSettingRebuildShowMode();

// �Ի�������
	enum { IDD = IDD_DLG_SHOW_MODE_REBUILD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	int m_showMode;
};
