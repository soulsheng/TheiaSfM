
// 3DRebuilderView.h : CMy3DRebuilderView 类的接口
//

#pragma once

#include <theia/theia.h>
#include "theia/io/read_ply_file.h"
#include "glCameraNode.h"
#include "FPSCounter.h"
#include <theia/theia.h>

using namespace theia;

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

	cameranode	m_camera;

	int		nColorPoint[3];
	int y_up_direction = -1;// 1-up,  -1-down 
	int min_num_views_for_track = 10;
	int n_fps = 30; // frame per second
	FPSCounter fps_mm;

	int MySetPixelFormat(HDC hdc);
	void cleanup();
	void initializeGL();

	void renderScene();
	void renderPointsCloud();
	void DrawColorBox(void);

	template<typename T>
	void getValueFromString(std::string str, T * cColor);

protected:
	// reconstruction
	Reconstruction* reconstruction = NULL;
	std::vector<Reconstruction*> reconstructions;

	std::string		m_imagePath;

	void build_reconstruction(std::vector<Reconstruction *>& reconstructions);
	void run_pmvs(const char *exeFullPath);
	void export_to_pmvs(theia::Reconstruction& reconstruction);

private:
	void CreateDirectoryIfDoesNotExist(const std::string& directory);
	int WriteCamerasToPMVS(const theia::Reconstruction& reconstruction);
	void WritePMVSOptions(const std::string& working_dir, const int num_images);
	void lanch_external_bin(String& bin, String& parameter, String& path, int nShowType = SW_HIDE);
	String getPath(String& strFullPath);
	void AddImagesToReconstructionBuilderDIY(ReconstructionBuilder* reconstruction_builder);

	void loadAndDisplayDenseResult();
	void loadAndDisplaySparseResult();

protected:
	// other 
	float step, s;
	bool	m_bDenseFinish;
	std::string m_strPathExe;
	int		m_nShowMode = SW_HIDE;

// 生成的消息映射函数
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnMenuViewPly();

	void outputInfo(int info, const char* message, bool bOutputORStatus = true);	// print to output window or status bar
	void outputInfo(const char* message, bool bOutputORStatus = true);	// print to output window or status bar

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSelectImagePath();
	afx_msg void OnExecuteReconstructionSparse();
	afx_msg void OnExecuteReconstructionDense();
	afx_msg void OnViewSparseResult();


	afx_msg void OnPrintScreen();

	void printScreen(std::string filename, int width = 1024, int height = 768);

	afx_msg void OnRebuildOneKey();
};

#ifndef _DEBUG  // 3DRebuilderView.cpp 中的调试版本
inline CMy3DRebuilderDoc* CMy3DRebuilderView::GetDocument() const
   { return reinterpret_cast<CMy3DRebuilderDoc*>(m_pDocument); }
#endif

template<typename T>
void CMy3DRebuilderView::getValueFromString(std::string str, T * cColor)
{
	std::istringstream in(str);
	char tmp;
	in >> tmp >> cColor[0] >> tmp >> cColor[1] >> tmp >> cColor[2];
}
