
// 3DRebuilderView.h : CMy3DRebuilderView ��Ľӿ�
//

#pragma once

#include <theia/theia.h>
#include "theia/io/read_ply_file.h"

class CMy3DRebuilderView : public CView
{
protected: // �������л�����
	CMy3DRebuilderView();
	DECLARE_DYNCREATE(CMy3DRebuilderView)

// ����
public:
	CMy3DRebuilderDoc* GetDocument() const;

// ����
public:

// ��д
public:
	virtual void OnDraw(CDC* pDC);  // ��д�Ի��Ƹ���ͼ
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// ʵ��
public:
	virtual ~CMy3DRebuilderView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	// point cloud
	theia::Vector3dVec world_points;
	theia::Vector3fVec point_colors;
	theia::Vector3fVec point_normals;
	std::vector<int> num_views_for_track;

	void rand_num_views_for_track(std::vector<int>& num_views_for_track, int size);

protected:
	// opengl
	HDC hdc;
	HGLRC hglrc;

	int MySetPixelFormat(HDC hdc);
	void cleanup();
	void initializeGL();

	void renderScene();
	void renderPointsCloud();
	void DrawColorBox(void);

protected:
	// other 
	float step, s;

// ���ɵ���Ϣӳ�亯��
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnMenuViewPly();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

#ifndef _DEBUG  // 3DRebuilderView.cpp �еĵ��԰汾
inline CMy3DRebuilderDoc* CMy3DRebuilderView::GetDocument() const
   { return reinterpret_cast<CMy3DRebuilderDoc*>(m_pDocument); }
#endif

