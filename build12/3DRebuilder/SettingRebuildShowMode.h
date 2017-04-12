#pragma once


// CSettingRebuildShowMode 对话框

class CSettingRebuildShowMode : public CDialogEx
{
	DECLARE_DYNAMIC(CSettingRebuildShowMode)

public:
	CSettingRebuildShowMode(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSettingRebuildShowMode();

// 对话框数据
	enum { IDD = IDD_DLG_SHOW_MODE_REBUILD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	int m_showMode;
};
