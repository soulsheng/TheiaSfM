
// 3DRebuilderView.h : CMy3DRebuilderView 类的接口
//

#pragma once

#include <theia/theia.h>
#include "theia/io/read_ply_file.h"

class CMy3DRebuilderView : public CView
{
protected: // 仅从序列化创建
	CMy3DRebuilderView();
	DECLARE_DYNCREATE(CMy3DRebuilderView)

// 特性
public:
	CMy3DRebuilderDoc* GetDocument() const;

// 操作
public:

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 实现
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

// 生成的消息映射函数
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

#ifndef _DEBUG  // 3DRebuilderView.cpp 中的调试版本
inline CMy3DRebuilderDoc* CMy3DRebuilderView::GetDocument() const
   { return reinterpret_cast<CMy3DRebuilderDoc*>(m_pDocument); }
#endif

