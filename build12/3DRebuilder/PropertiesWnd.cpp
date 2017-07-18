
#include "stdafx.h"

#include "PropertiesWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "3DRebuilder.h"

#include "3DRebuilderDoc.h"
#include "3DRebuilderView.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar

CPropertiesWnd::CPropertiesWnd()
{
	//m_nComboHeight = 0;
}

CPropertiesWnd::~CPropertiesWnd()
{
}

BEGIN_MESSAGE_MAP(CPropertiesWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_EXPAND_ALL, OnExpandAllProperties)
	ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, OnUpdateExpandAllProperties)
	ON_COMMAND(ID_SORTPROPERTIES, OnSortProperties)
	ON_UPDATE_COMMAND_UI(ID_SORTPROPERTIES, OnUpdateSortProperties)
	ON_COMMAND(ID_PROPERTIES1, OnProperties1)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES1, OnUpdateProperties1)
	ON_COMMAND(ID_PROPERTIES2, OnProperties2)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES2, OnUpdateProperties2)
	ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, OnPropertyChanged)
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar 消息处理程序

void CPropertiesWnd::AdjustLayout()
{
	if (GetSafeHwnd () == NULL || (AfxGetMainWnd() != NULL && AfxGetMainWnd()->IsIconic()))
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	//m_wndObjectCombo.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), m_nComboHeight, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top , rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndPropList.SetWindowPos(NULL, rectClient.left, rectClient.top + cyTlb, rectClient.Width(), rectClient.Height() -cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
}

int CPropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();
#if 0
	// 创建组合: 
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | CBS_SORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (!m_wndObjectCombo.Create(dwViewStyle, rectDummy, this, 1))
	{
		TRACE0("未能创建属性组合 \n");
		return -1;      // 未能创建
	}

	m_wndObjectCombo.AddString(_T("应用程序"));
	m_wndObjectCombo.AddString(_T("属性窗口"));
	m_wndObjectCombo.SetCurSel(0);

	CRect rectCombo;
	m_wndObjectCombo.GetClientRect (&rectCombo);

	m_nComboHeight = rectCombo.Height();
#endif
	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2))
	{
		TRACE0("未能创建属性网格\n");
		return -1;      // 未能创建
	}

	InitPropList();

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	m_wndToolBar.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* 已锁定*/);
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* 锁定*/);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);

	// 所有命令将通过此控件路由，而不是通过主框架路由: 
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	AdjustLayout();
	return 0;
}

void CPropertiesWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CPropertiesWnd::OnExpandAllProperties()
{
	m_wndPropList.ExpandAll();
}

void CPropertiesWnd::OnUpdateExpandAllProperties(CCmdUI* /* pCmdUI */)
{
}

void CPropertiesWnd::OnSortProperties()
{
	m_wndPropList.SetAlphabeticMode(!m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::OnUpdateSortProperties(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::OnProperties1()
{
	// TODO:  在此处添加命令处理程序代码
}

void CPropertiesWnd::OnUpdateProperties1(CCmdUI* /*pCmdUI*/)
{
	// TODO:  在此处添加命令更新 UI 处理程序代码
}

void CPropertiesWnd::OnProperties2()
{
	// TODO:  在此处添加命令处理程序代码
}

void CPropertiesWnd::OnUpdateProperties2(CCmdUI* /*pCmdUI*/)
{
	// TODO:  在此处添加命令更新 UI 处理程序代码
}

void CPropertiesWnd::InitPropList()
{
	SetPropListFont();

	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();

#if 0 // 外观
	CMFCPropertyGridProperty* pGroup1 = new CMFCPropertyGridProperty(_T("外观"));

	pGroup1->AddSubItem(new CMFCPropertyGridProperty(_T("三维外观"), (_variant_t) false, _T("指定窗口的字体不使用粗体，并且控件将使用三维边框")));

	CMFCPropertyGridProperty* pProp = new CMFCPropertyGridProperty(_T("边框"), _T("对话框外框"), _T("其中之一: “无”、“细”、“可调整大小”或“对话框外框”"));
	pProp->AddOption(_T("无"));
	pProp->AddOption(_T("细"));
	pProp->AddOption(_T("可调整大小"));
	pProp->AddOption(_T("对话框外框"));
	pProp->AllowEdit(FALSE);

	pGroup1->AddSubItem(pProp);
	pGroup1->AddSubItem(new CMFCPropertyGridProperty(_T("标题"), (_variant_t) _T("关于"), _T("指定窗口标题栏中显示的文本")));

	m_wndPropList.AddProperty(pGroup1);
#endif

#if 0 // 窗口大小

	CMFCPropertyGridProperty* pSize = new CMFCPropertyGridProperty(_T("窗口大小"), 0, TRUE);

	pProp = new CMFCPropertyGridProperty(_T("高度"), (_variant_t) 250l, _T("指定窗口的高度"));
	pProp->EnableSpinControl(TRUE, 50, 300);
	pSize->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("宽度"), (_variant_t) 150l, _T("指定窗口的宽度"));
	pProp->EnableSpinControl(TRUE, 50, 200);
	pSize->AddSubItem(pProp);

	m_wndPropList.AddProperty(pSize);
#endif

#if 0 // 字体
	CMFCPropertyGridProperty* pGroup2 = new CMFCPropertyGridProperty(_T("字体"));

	LOGFONT lf;
	CFont* font = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	font->GetLogFont(&lf);

	lstrcpy(lf.lfFaceName, _T("宋体, Arial"));

	pGroup2->AddSubItem(new CMFCPropertyGridFontProperty(_T("字体"), lf, CF_EFFECTS | CF_SCREENFONTS, _T("指定窗口的默认字体")));
	pGroup2->AddSubItem(new CMFCPropertyGridProperty(_T("使用系统字体"), (_variant_t) true, _T("指定窗口使用“MS Shell Dlg”字体")));

	m_wndPropList.AddProperty(pGroup2);
#endif

#if 0 // 杂项
	CMFCPropertyGridProperty* pGroup3 = new CMFCPropertyGridProperty(_T("杂项"));
	pProp = new CMFCPropertyGridProperty(_T("(名称)"), _T("应用程序"));
	pProp->Enable(FALSE);
	pGroup3->AddSubItem(pProp);

	CMFCPropertyGridColorProperty* pColorProp = new CMFCPropertyGridColorProperty(_T("窗口颜色"), RGB(210, 192, 254), NULL, _T("指定默认的窗口颜色"));
	pColorProp->EnableOtherButton(_T("其他..."));
	pColorProp->EnableAutomaticButton(_T("默认"), ::GetSysColor(COLOR_3DFACE));
	pGroup3->AddSubItem(pColorProp);

	static const TCHAR szFilter[] = _T("图标文件(*.ico)|*.ico|所有文件(*.*)|*.*||");
	pGroup3->AddSubItem(new CMFCPropertyGridFileProperty(_T("图标"), TRUE, _T(""), _T("ico"), 0, szFilter, _T("指定窗口图标")));

	pGroup3->AddSubItem(new CMFCPropertyGridFileProperty(_T("文件夹"), _T("c:\\")));

	m_wndPropList.AddProperty(pGroup3);
#endif

#if 0 // 层次结构
	CMFCPropertyGridProperty* pGroup4 = new CMFCPropertyGridProperty(_T("层次结构"));

	CMFCPropertyGridProperty* pGroup41 = new CMFCPropertyGridProperty(_T("第一个子级"));
	pGroup4->AddSubItem(pGroup41);

	CMFCPropertyGridProperty* pGroup411 = new CMFCPropertyGridProperty(_T("第二个子级"));
	pGroup41->AddSubItem(pGroup411);

	pGroup411->AddSubItem(new CMFCPropertyGridProperty(_T("项 1"), (_variant_t) _T("值 1"), _T("此为说明")));
	pGroup411->AddSubItem(new CMFCPropertyGridProperty(_T("项 2"), (_variant_t) _T("值 2"), _T("此为说明")));
	pGroup411->AddSubItem(new CMFCPropertyGridProperty(_T("项 3"), (_variant_t) _T("值 3"), _T("此为说明")));

	pGroup4->Expand(FALSE);
	m_wndPropList.AddProperty(pGroup4);
#endif

#if 0
	CMFCPropertyGridProperty* pProp = NULL;

	// 窗口大小

	//CMFCPropertyGridProperty* pSize = new CMFCPropertyGridProperty(_T("窗口大小"), ID_PROPERTIES1, TRUE);

	pProp = new CMFCPropertyGridProperty(_T("高度"), (_variant_t)960, _T("指定窗口的高度"), ID_PROPERTIES1);
	pProp->EnableSpinControl(TRUE, 50, 2048);
	//pSize->AddSubItem(pProp);
	m_wndPropList.AddProperty(pProp);

	pProp = new CMFCPropertyGridProperty(_T("宽度"), (_variant_t)1280, _T("指定窗口的宽度"), ID_PROPERTIES2);
	pProp->EnableSpinControl(TRUE, 50, 2048);
	//pSize->AddSubItem(pProp);

	m_wndPropList.AddProperty(pProp);
#endif

	CMFCPropertyGridProperty* pProp = NULL;

	CMFCPropertyGridProperty* pGroup = NULL;

	// 分组1 - 重建 
	pGroup = new CMFCPropertyGridProperty(_T("重建"));

	// 点云统一颜色
	pProp = new CMFCPropertyGridProperty(_T("是否重建"), (_variant_t)1, _T("是否重新重建"), 0);
	pProp->EnableSpinControl(TRUE, 0, 1);
	pGroup->AddSubItem(pProp);

	m_wndPropList.AddProperty(pGroup);

	// 分组2 - 显示 
	pGroup = new CMFCPropertyGridProperty(_T("显示"));

	// 背景颜色
	CMFCPropertyGridColorProperty* pColorPropBG = new CMFCPropertyGridColorProperty(_T("背景颜色"), RGB(128, 150, 200), NULL, _T("指定背景颜色，默认天蓝色"));
	pColorPropBG->EnableOtherButton(_T("其他..."));
	pColorPropBG->EnableAutomaticButton(_T("默认"), ::GetSysColor(COLOR_3DFACE));
	//pGroup3->AddSubItem(pColorProp);
	pGroup->AddSubItem(pColorPropBG);


	// 点云颜色
	CMFCPropertyGridColorProperty* pColorPropPoint = new CMFCPropertyGridColorProperty(_T("点云颜色"), RGB(0, 250, 0), NULL, _T("点云颜色，默认绿色"));
	pColorPropPoint->EnableOtherButton(_T("其他..."));
	pColorPropPoint->EnableAutomaticButton(_T("默认"), ::GetSysColor(COLOR_3DFACE));
	//pGroup3->AddSubItem(pColorProp);
	pGroup->AddSubItem(pColorPropPoint);

	// 点云统一颜色
	pProp = new CMFCPropertyGridProperty(_T("统一颜色"), (_variant_t)1, _T("点云采用统一颜色"), 0);
	pProp->EnableSpinControl(TRUE, 0, 1);
	pGroup->AddSubItem(pProp);

	// 点的大小
	pProp = new CMFCPropertyGridProperty(_T("点的大小"), (_variant_t)3, _T("绘制点的大小"), 0);
	pProp->EnableSpinControl(TRUE, 1, 10);
	pGroup->AddSubItem(pProp);

	m_wndPropList.AddProperty(pGroup);


	// 分组3 - 输出 
	pGroup = new CMFCPropertyGridProperty(_T("输出"));

	// 格式
	pProp = new CMFCPropertyGridProperty(_T("格式"), (_variant_t)"jpg+gif+avi ", _T("输出格式，例如：jpg+gif+avi"), 0);
	pGroup->AddSubItem(pProp);

	// fps 帧速
	pProp = new CMFCPropertyGridProperty(_T("帧速fps"), (_variant_t)2, _T("设置输出动画的帧速，默认：每秒2帧"), 0);
	pProp->EnableSpinControl(TRUE, 1, 100);
	pGroup->AddSubItem(pProp);

	// 文件名
	pProp = new CMFCPropertyGridProperty(_T("文件名"), (_variant_t)"0abc", _T("设置动画的文件名，默认：0.gif 0.avi"), 0);
	pGroup->AddSubItem(pProp);

	// 时长
	pProp = new CMFCPropertyGridProperty(_T("时长"), (_variant_t)5, _T("设置输出动画的时间长度，默认：5秒"), 0);
	pProp->EnableSpinControl(TRUE, 1, 100);
	pGroup->AddSubItem(pProp);

	m_wndPropList.AddProperty(pGroup);

}

void CPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndPropList.SetFocus();
}

void CPropertiesWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);
	SetPropListFont();
}

void CPropertiesWnd::SetPropListFont()
{
	::DeleteObject(m_fntPropList.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	m_fntPropList.CreateFontIndirect(&lf);

	m_wndPropList.SetFont(&m_fntPropList);
	//m_wndObjectCombo.SetFont(&m_fntPropList);
}


LRESULT CPropertiesWnd::OnPropertyChanged(
	__in WPARAM wparam,
	__in LPARAM lparam)
{
	CMFCPropertyGridProperty* pProp = reinterpret_cast<CMFCPropertyGridProperty*>(lparam);

	if (NULL == pProp)
		return 0;

	int nID = pProp->GetData();
	CString name = pProp->GetName();
	CString value = pProp->GetValue();

	CMainFrame* pFrame = (CMainFrame *)AfxGetMainWnd();
	CMy3DRebuilderView* pView = (CMy3DRebuilderView *)pFrame->GetActiveView();

	CRect rect;	// 存储当前窗口

	if (name == "背景颜色")
	{
		COLORREF color = ((CMFCPropertyGridColorProperty*)pProp)->GetColor();
		float r = GetRValue(color);
		float g = GetGValue(color);
		float b = GetBValue(color);
		pView->setColorBG(r, g, b);
	}

	if (name == "点云颜色")
	{
		COLORREF color = ((CMFCPropertyGridColorProperty*)pProp)->GetColor();
		float r = GetRValue(color);
		float g = GetGValue(color);
		float b = GetBValue(color);
		pView->setColorPoint(r, g, b);
	}

	if (name == "统一颜色")
	{
		int bFlag = _ttoi(value);
		pView->setColorPointFlag(bFlag);
	}

	if (name == "点的大小")
	{
		int nSize = _ttoi(value);
		pView->setSizePoint(nSize);
	}

	if (name == "是否重建")
	{
		int bFlag = _ttoi(value);
		pView->setBuildFlag(bFlag);
	}

	if (name == "格式")
	{
		pView->setOutputFormat(value.GetBuffer());
	}

	if (name == "帧速fps")
	{
		int nSize = _ttoi(value);
		pView->setOutputFPS(nSize);
	}

	if (name == "文件名")
	{
		pView->setOutputName(value.GetBuffer());
	}

	if (name == "时长")
	{
		int nSize = _ttoi(value);
		pView->setOutputLength(nSize);
	}

	return(0);
}