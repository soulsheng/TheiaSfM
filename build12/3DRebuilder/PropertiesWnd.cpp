
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
// CResourceViewBar ��Ϣ�������

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
	// �������: 
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | CBS_SORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (!m_wndObjectCombo.Create(dwViewStyle, rectDummy, this, 1))
	{
		TRACE0("δ�ܴ���������� \n");
		return -1;      // δ�ܴ���
	}

	m_wndObjectCombo.AddString(_T("Ӧ�ó���"));
	m_wndObjectCombo.AddString(_T("���Դ���"));
	m_wndObjectCombo.SetCurSel(0);

	CRect rectCombo;
	m_wndObjectCombo.GetClientRect (&rectCombo);

	m_nComboHeight = rectCombo.Height();
#endif
	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2))
	{
		TRACE0("δ�ܴ�����������\n");
		return -1;      // δ�ܴ���
	}

	InitPropList();

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	m_wndToolBar.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* ������*/);
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* ����*/);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);

	// �������ͨ���˿ؼ�·�ɣ�������ͨ�������·��: 
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
	// TODO:  �ڴ˴���������������
}

void CPropertiesWnd::OnUpdateProperties1(CCmdUI* /*pCmdUI*/)
{
	// TODO:  �ڴ˴����������� UI ����������
}

void CPropertiesWnd::OnProperties2()
{
	// TODO:  �ڴ˴���������������
}

void CPropertiesWnd::OnUpdateProperties2(CCmdUI* /*pCmdUI*/)
{
	// TODO:  �ڴ˴����������� UI ����������
}

void CPropertiesWnd::InitPropList()
{
	SetPropListFont();

	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();

#if 0 // ���
	CMFCPropertyGridProperty* pGroup1 = new CMFCPropertyGridProperty(_T("���"));

	pGroup1->AddSubItem(new CMFCPropertyGridProperty(_T("��ά���"), (_variant_t) false, _T("ָ�����ڵ����岻ʹ�ô��壬���ҿؼ���ʹ����ά�߿�")));

	CMFCPropertyGridProperty* pProp = new CMFCPropertyGridProperty(_T("�߿�"), _T("�Ի������"), _T("����֮һ: ���ޡ�����ϸ�������ɵ�����С���򡰶Ի������"));
	pProp->AddOption(_T("��"));
	pProp->AddOption(_T("ϸ"));
	pProp->AddOption(_T("�ɵ�����С"));
	pProp->AddOption(_T("�Ի������"));
	pProp->AllowEdit(FALSE);

	pGroup1->AddSubItem(pProp);
	pGroup1->AddSubItem(new CMFCPropertyGridProperty(_T("����"), (_variant_t) _T("����"), _T("ָ�����ڱ���������ʾ���ı�")));

	m_wndPropList.AddProperty(pGroup1);
#endif

#if 0 // ���ڴ�С

	CMFCPropertyGridProperty* pSize = new CMFCPropertyGridProperty(_T("���ڴ�С"), 0, TRUE);

	pProp = new CMFCPropertyGridProperty(_T("�߶�"), (_variant_t) 250l, _T("ָ�����ڵĸ߶�"));
	pProp->EnableSpinControl(TRUE, 50, 300);
	pSize->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("���"), (_variant_t) 150l, _T("ָ�����ڵĿ��"));
	pProp->EnableSpinControl(TRUE, 50, 200);
	pSize->AddSubItem(pProp);

	m_wndPropList.AddProperty(pSize);
#endif

#if 0 // ����
	CMFCPropertyGridProperty* pGroup2 = new CMFCPropertyGridProperty(_T("����"));

	LOGFONT lf;
	CFont* font = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	font->GetLogFont(&lf);

	lstrcpy(lf.lfFaceName, _T("����, Arial"));

	pGroup2->AddSubItem(new CMFCPropertyGridFontProperty(_T("����"), lf, CF_EFFECTS | CF_SCREENFONTS, _T("ָ�����ڵ�Ĭ������")));
	pGroup2->AddSubItem(new CMFCPropertyGridProperty(_T("ʹ��ϵͳ����"), (_variant_t) true, _T("ָ������ʹ�á�MS Shell Dlg������")));

	m_wndPropList.AddProperty(pGroup2);
#endif

#if 0 // ����
	CMFCPropertyGridProperty* pGroup3 = new CMFCPropertyGridProperty(_T("����"));
	pProp = new CMFCPropertyGridProperty(_T("(����)"), _T("Ӧ�ó���"));
	pProp->Enable(FALSE);
	pGroup3->AddSubItem(pProp);

	CMFCPropertyGridColorProperty* pColorProp = new CMFCPropertyGridColorProperty(_T("������ɫ"), RGB(210, 192, 254), NULL, _T("ָ��Ĭ�ϵĴ�����ɫ"));
	pColorProp->EnableOtherButton(_T("����..."));
	pColorProp->EnableAutomaticButton(_T("Ĭ��"), ::GetSysColor(COLOR_3DFACE));
	pGroup3->AddSubItem(pColorProp);

	static const TCHAR szFilter[] = _T("ͼ���ļ�(*.ico)|*.ico|�����ļ�(*.*)|*.*||");
	pGroup3->AddSubItem(new CMFCPropertyGridFileProperty(_T("ͼ��"), TRUE, _T(""), _T("ico"), 0, szFilter, _T("ָ������ͼ��")));

	pGroup3->AddSubItem(new CMFCPropertyGridFileProperty(_T("�ļ���"), _T("c:\\")));

	m_wndPropList.AddProperty(pGroup3);
#endif

#if 0 // ��νṹ
	CMFCPropertyGridProperty* pGroup4 = new CMFCPropertyGridProperty(_T("��νṹ"));

	CMFCPropertyGridProperty* pGroup41 = new CMFCPropertyGridProperty(_T("��һ���Ӽ�"));
	pGroup4->AddSubItem(pGroup41);

	CMFCPropertyGridProperty* pGroup411 = new CMFCPropertyGridProperty(_T("�ڶ����Ӽ�"));
	pGroup41->AddSubItem(pGroup411);

	pGroup411->AddSubItem(new CMFCPropertyGridProperty(_T("�� 1"), (_variant_t) _T("ֵ 1"), _T("��Ϊ˵��")));
	pGroup411->AddSubItem(new CMFCPropertyGridProperty(_T("�� 2"), (_variant_t) _T("ֵ 2"), _T("��Ϊ˵��")));
	pGroup411->AddSubItem(new CMFCPropertyGridProperty(_T("�� 3"), (_variant_t) _T("ֵ 3"), _T("��Ϊ˵��")));

	pGroup4->Expand(FALSE);
	m_wndPropList.AddProperty(pGroup4);
#endif

#if 0
	CMFCPropertyGridProperty* pProp = NULL;

	// ���ڴ�С

	//CMFCPropertyGridProperty* pSize = new CMFCPropertyGridProperty(_T("���ڴ�С"), ID_PROPERTIES1, TRUE);

	pProp = new CMFCPropertyGridProperty(_T("�߶�"), (_variant_t)960, _T("ָ�����ڵĸ߶�"), ID_PROPERTIES1);
	pProp->EnableSpinControl(TRUE, 50, 2048);
	//pSize->AddSubItem(pProp);
	m_wndPropList.AddProperty(pProp);

	pProp = new CMFCPropertyGridProperty(_T("���"), (_variant_t)1280, _T("ָ�����ڵĿ��"), ID_PROPERTIES2);
	pProp->EnableSpinControl(TRUE, 50, 2048);
	//pSize->AddSubItem(pProp);

	m_wndPropList.AddProperty(pProp);
#endif

	CMFCPropertyGridProperty* pProp = NULL;

	CMFCPropertyGridProperty* pGroup = NULL;

	// ����1 - �ؽ� 
	pGroup = new CMFCPropertyGridProperty(_T("�ؽ�"));

	// ����ͳһ��ɫ
	pProp = new CMFCPropertyGridProperty(_T("�Ƿ��ؽ�"), (_variant_t)1, _T("�Ƿ������ؽ�"), 0);
	pProp->EnableSpinControl(TRUE, 0, 1);
	pGroup->AddSubItem(pProp);

	m_wndPropList.AddProperty(pGroup);

	// ����2 - ��ʾ 
	pGroup = new CMFCPropertyGridProperty(_T("��ʾ"));

	// ������ɫ
	CMFCPropertyGridColorProperty* pColorPropBG = new CMFCPropertyGridColorProperty(_T("������ɫ"), RGB(128, 150, 200), NULL, _T("ָ��������ɫ��Ĭ������ɫ"));
	pColorPropBG->EnableOtherButton(_T("����..."));
	pColorPropBG->EnableAutomaticButton(_T("Ĭ��"), ::GetSysColor(COLOR_3DFACE));
	//pGroup3->AddSubItem(pColorProp);
	pGroup->AddSubItem(pColorPropBG);


	// ������ɫ
	CMFCPropertyGridColorProperty* pColorPropPoint = new CMFCPropertyGridColorProperty(_T("������ɫ"), RGB(0, 250, 0), NULL, _T("������ɫ��Ĭ����ɫ"));
	pColorPropPoint->EnableOtherButton(_T("����..."));
	pColorPropPoint->EnableAutomaticButton(_T("Ĭ��"), ::GetSysColor(COLOR_3DFACE));
	//pGroup3->AddSubItem(pColorProp);
	pGroup->AddSubItem(pColorPropPoint);

	// ����ͳһ��ɫ
	pProp = new CMFCPropertyGridProperty(_T("ͳһ��ɫ"), (_variant_t)1, _T("���Ʋ���ͳһ��ɫ"), 0);
	pProp->EnableSpinControl(TRUE, 0, 1);
	pGroup->AddSubItem(pProp);

	// ��Ĵ�С
	pProp = new CMFCPropertyGridProperty(_T("��Ĵ�С"), (_variant_t)3, _T("���Ƶ�Ĵ�С"), 0);
	pProp->EnableSpinControl(TRUE, 1, 10);
	pGroup->AddSubItem(pProp);

	m_wndPropList.AddProperty(pGroup);


	// ����3 - ��� 
	pGroup = new CMFCPropertyGridProperty(_T("���"));

	// ��ʽ
	pProp = new CMFCPropertyGridProperty(_T("��ʽ"), (_variant_t)"jpg+gif+avi ", _T("�����ʽ�����磺jpg+gif+avi"), 0);
	pGroup->AddSubItem(pProp);

	// fps ֡��
	pProp = new CMFCPropertyGridProperty(_T("֡��fps"), (_variant_t)2, _T("�������������֡�٣�Ĭ�ϣ�ÿ��2֡"), 0);
	pProp->EnableSpinControl(TRUE, 1, 100);
	pGroup->AddSubItem(pProp);

	// �ļ���
	pProp = new CMFCPropertyGridProperty(_T("�ļ���"), (_variant_t)"0abc", _T("���ö������ļ�����Ĭ�ϣ�0.gif 0.avi"), 0);
	pGroup->AddSubItem(pProp);

	// ʱ��
	pProp = new CMFCPropertyGridProperty(_T("ʱ��"), (_variant_t)5, _T("�������������ʱ�䳤�ȣ�Ĭ�ϣ�5��"), 0);
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

	CRect rect;	// �洢��ǰ����

	if (name == "������ɫ")
	{
		COLORREF color = ((CMFCPropertyGridColorProperty*)pProp)->GetColor();
		float r = GetRValue(color);
		float g = GetGValue(color);
		float b = GetBValue(color);
		pView->setColorBG(r, g, b);
	}

	if (name == "������ɫ")
	{
		COLORREF color = ((CMFCPropertyGridColorProperty*)pProp)->GetColor();
		float r = GetRValue(color);
		float g = GetGValue(color);
		float b = GetBValue(color);
		pView->setColorPoint(r, g, b);
	}

	if (name == "ͳһ��ɫ")
	{
		int bFlag = _ttoi(value);
		pView->setColorPointFlag(bFlag);
	}

	if (name == "��Ĵ�С")
	{
		int nSize = _ttoi(value);
		pView->setSizePoint(nSize);
	}

	if (name == "�Ƿ��ؽ�")
	{
		int bFlag = _ttoi(value);
		pView->setBuildFlag(bFlag);
	}

	if (name == "��ʽ")
	{
		pView->setOutputFormat(value.GetBuffer());
	}

	if (name == "֡��fps")
	{
		int nSize = _ttoi(value);
		pView->setOutputFPS(nSize);
	}

	if (name == "�ļ���")
	{
		pView->setOutputName(value.GetBuffer());
	}

	if (name == "ʱ��")
	{
		int nSize = _ttoi(value);
		pView->setOutputLength(nSize);
	}

	return(0);
}