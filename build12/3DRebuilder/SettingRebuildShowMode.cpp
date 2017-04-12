// SettingRebuildShowMode.cpp : 实现文件
//

#include "stdafx.h"
#include "3DRebuilder.h"
#include "SettingRebuildShowMode.h"
#include "afxdialogex.h"


// CSettingRebuildShowMode 对话框

IMPLEMENT_DYNAMIC(CSettingRebuildShowMode, CDialogEx)

CSettingRebuildShowMode::CSettingRebuildShowMode(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSettingRebuildShowMode::IDD, pParent)
	, m_showMode(0)
{

}

CSettingRebuildShowMode::~CSettingRebuildShowMode()
{
}

void CSettingRebuildShowMode::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_CBIndex(pDX, IDC_COMBO1, m_showMode);
}


BEGIN_MESSAGE_MAP(CSettingRebuildShowMode, CDialogEx)
END_MESSAGE_MAP()


// CSettingRebuildShowMode 消息处理程序
