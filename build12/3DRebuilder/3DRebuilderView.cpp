
// 3DRebuilderView.cpp : CMy3DRebuilderView 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "3DRebuilder.h"
#endif

#include "3DRebuilderDoc.h"
#include "3DRebuilderView.h"

#include <gflags/gflags.h>

#include "build_common.h"

#include "bmpHeader.h"
#include "bmp2gif.h"

#include "SettingRebuildShowMode.h"

#include "utility_theia.h"

#include "LaunchPMVS2.h"

DEFINE_bool(same_color_point, true, "bool on/off to use same color for point. eg:0 ");
DEFINE_int32(draw_point_size, 1, "bool on/off to use same color for point. eg:0 ");
DEFINE_string(color_sky, "(128,150,200)", "color of sky. eg:(128,150,200)blue ");
DEFINE_string(color_point, "(0,255,0)", "color of point. eg:(255,255,255)white ");
DEFINE_string(ply_file, "option-0000.ply", "Output PLY file.");
DEFINE_string(image_directory, "",
	"Full path to the directory containing the images used to create "
	"the reconstructions. Must contain a trailing slash.");

DEFINE_string(pmvs_working_directory, "",
	"A directory to store the necessary pmvs files.");

DEFINE_string(eye_position, "(-50,180,550)", "position of eye.");
DEFINE_bool(undistort, false, "bool on/off to undistort image. eg:0 ");

DEFINE_string(output_image_directory, "E:\\3d\\output\\", "output image directory");
DEFINE_int32(output_image_type, 1, "0 bmp, 1 gif, 2 mp4 ");
DEFINE_string(distance, "(0.1,0.6,0.2)", "set distance of view");
DEFINE_int32(view_type, 0, "0-perspective, 1-camera, 2-top, 3-free, 4-common");
DEFINE_bool(swap_yz, false, "swap y and z");
DEFINE_bool(draw_box, false, "draw bounding box");
DEFINE_int32(threshold_group, 35, "threshodGroup to filter group of outlier points.");

DEFINE_string(format, "jpg+gif+avi+mp4", "jpg, gif, avi, mp4 ");
DEFINE_int32(width, 1280, "window width");
DEFINE_int32(height, 1024, "window height");
DEFINE_double(fps, 2, "frame per second");
DEFINE_string(name, "0abc", "gif or mp4 file name");
DEFINE_double(length, 5, "length of vedio, unit: seconds");

DEFINE_bool(build, true, "bool on/off to build. eg:0 ");

//#define	FLAGS_ply_file	"option-0000.ply"
#define CLIP_FAR_DISTANCE	100000	// 10000

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIME_DRAW		1

// CMy3DRebuilderView

IMPLEMENT_DYNCREATE(CMy3DRebuilderView, CView)

BEGIN_MESSAGE_MAP(CMy3DRebuilderView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CMy3DRebuilderView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_Menu_View_PLY, &CMy3DRebuilderView::OnMenuViewPly)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
	ON_COMMAND(ID_SELECT_IMAGE_PATH, &CMy3DRebuilderView::OnSelectImagePath)
	ON_COMMAND(ID_EXECUTE_RECONSTRUCTION_SPARSE, &CMy3DRebuilderView::OnExecuteReconstructionSparse)
	ON_COMMAND(ID_EXECUTE_RECONSTRUCTION_DENSE, &CMy3DRebuilderView::OnExecuteReconstructionDense)
	ON_COMMAND(ID_VIEW_SPARSE_RESULT, &CMy3DRebuilderView::OnViewSparseResult)
	ON_COMMAND(ID_PRINT_SCREEN, &CMy3DRebuilderView::OnPrintScreen)
	ON_COMMAND(ID_REBUILD_ONE_KEY, &CMy3DRebuilderView::OnRebuildOneKey)
	ON_COMMAND(ID_MENU_OPEN_MANUAL, &CMy3DRebuilderView::OnOpenUserManual)
END_MESSAGE_MAP()

// CMy3DRebuilderView 构造/析构

CMy3DRebuilderView::CMy3DRebuilderView()
{
	// TODO:  在此处添加构造代码
	step = 0.0;
	s = 0.1;
	m_bDenseFinish = false;
}

CMy3DRebuilderView::~CMy3DRebuilderView()
{
	cleanup();
}

BOOL CMy3DRebuilderView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO:  在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CMy3DRebuilderView 绘制

void CMy3DRebuilderView::OnDraw(CDC* /*pDC*/)
{
	CMy3DRebuilderDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO:  在此处为本机数据添加绘制代码
	renderScene();

}


// CMy3DRebuilderView 打印


void CMy3DRebuilderView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CMy3DRebuilderView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CMy3DRebuilderView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO:  添加额外的打印前进行的初始化过程
}

void CMy3DRebuilderView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO:  添加打印后进行的清理过程
}

void CMy3DRebuilderView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CMy3DRebuilderView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CMy3DRebuilderView 诊断

#ifdef _DEBUG
void CMy3DRebuilderView::AssertValid() const
{
	CView::AssertValid();
}

void CMy3DRebuilderView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMy3DRebuilderDoc* CMy3DRebuilderView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMy3DRebuilderDoc)));
	return (CMy3DRebuilderDoc*)m_pDocument;
}
#endif //_DEBUG


// CMy3DRebuilderView 消息处理程序


void CMy3DRebuilderView::rand_num_views_for_track(std::vector<int>& num_views_for_track, int size)
{
	num_views_for_track.reserve(size);
	for (int i = 0; i < size; i++)
		num_views_for_track.emplace_back(rand() % min_num_views_for_track);
}

void CMy3DRebuilderView::OnMenuViewPly()
{
	// TODO:  在此添加命令处理程序代码
	loadAndDisplayDenseResult();


}


void CMy3DRebuilderView::loadAndDisplayDenseResult()
{
	if (!updateImagePath())
		return;

	world_points.clear();
	point_normals.clear();
	point_colors.clear();

	outputInfo("正在显示稠密重建结果...");

	if (!theia::ReadPlyFile(FLAGS_ply_file, world_points, point_normals, point_colors))
		printf("can not open ply file!\n");

	min_num_views_for_track = FLAGS_fps * FLAGS_length;
		
	rand_num_views_for_track(num_views_for_track, world_points.size());

	box.calculate(world_points, FLAGS_swap_yz);

	FLAGS_view_type = VIEW_PERSPECTIVE;

	updateEyePosition();

	outputInfo(world_points.size(), "稠密重建三维点的数目为：" );
	LOG(INFO) << "稠密重建三维点的数目为：" << world_points.size();

	outputInfo("稠密重建结果显示完成...");

	m_bDenseFinish = true;

}


void CMy3DRebuilderView::outputInfo(int info, const char* message, bool bOutputORStatus)
{
	CMainFrame* pMFram = (CMainFrame*)AfxGetMainWnd();

	std::ostringstream os;
	os << message << info ;
	if (bOutputORStatus)
		pMFram->FillBuildWindow(os.str());
	else
	{
		char* p_status = pMFram->getCustomStatusString();
		memset(p_status, '\0', sizeof(p_status));
		strncpy(p_status, os.str().c_str(), os.str().size());
	}

}


void CMy3DRebuilderView::outputInfo(const char* message, bool bOutputORStatus /*= true*/)
{
	char szTime[32];
	CTime time;
	time = CTime::GetCurrentTime();
	sprintf(szTime, "%4d-%.2d-%.2d %.2d:%.2d:%.2d",
		time.GetYear(), time.GetMonth(), time.GetDay(),
		time.GetHour(), time.GetMinute(), time.GetSecond());

	std::string strMessage(szTime);
	strMessage += message;

	CMainFrame* pMFram = (CMainFrame*)AfxGetMainWnd();
	if (bOutputORStatus)
		pMFram->FillBuildWindow(strMessage);
	else
	{
		char* p_status = pMFram->getCustomStatusString();
		memset(p_status, '\0', sizeof(p_status));
		strncpy(p_status, strMessage.c_str(), strMessage.size());
	}
	Sleep(100);
}

int CMy3DRebuilderView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	initializeGL();

	// 设置计时器,10ms刷新一次
	SetTimer(TIME_DRAW, 10, 0);

	getValueFromString(std::string(FLAGS_color_point), nColorPoint);

	int temp[3];
	getValueFromString(std::string(FLAGS_eye_position), m_camera.getEye());

	HINSTANCE hInst = AfxGetApp()->m_hInstance;
	char path_buffer[_MAX_PATH];
	GetModuleFileName(hInst, path_buffer, sizeof(path_buffer));//得到exe文件的全路径

	m_strPathExe = getPath(std::string(path_buffer) );

	return 0;
}

int CMy3DRebuilderView::MySetPixelFormat(HDC hdc)
{
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR), // pfd结构的大小 
		1, // 版本号 
		PFD_DRAW_TO_WINDOW | // 支持在窗口中绘图 
		PFD_SUPPORT_OPENGL | // 支持 OpenGL 
		PFD_DOUBLEBUFFER, // 双缓存模式 
		PFD_TYPE_RGBA, // RGBA 颜色模式 
		24, // 24 位颜色深度 
		0, 0, 0, 0, 0, 0, // 忽略颜色位 
		0, // 没有非透明度缓存 
		0, // 忽略移位位 
		0, // 无累加缓存 
		0, 0, 0, 0, // 忽略累加位 
		32, // 32 位深度缓存 
		0, // 无模板缓存 
		0, // 无辅助缓存 
		PFD_MAIN_PLANE, // 主层 
		0, // 保留 
		0, 0, 0 // 忽略层,可见性和损毁掩模 
	};

	int iPixelFormat;

	// 为设备描述表得到最匹配的像素格式 
	if ((iPixelFormat = ChoosePixelFormat(hdc, &pfd)) == 0)
	{
		MessageBox("ChoosePixelFormat Failed", NULL, MB_OK);
		return 0;
	}

	// 设置最匹配的像素格式为当前的像素格式 
	if (SetPixelFormat(hdc, iPixelFormat, &pfd) == FALSE)
	{
		MessageBox("SetPixelFormat Failed", NULL, MB_OK);
		return 0;
	}

	return 1;
}

void CMy3DRebuilderView::cleanup()
{

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hglrc);//删除渲染描述表
	::ReleaseDC(m_hWnd, hdc);//释放设备描述表

}

void CMy3DRebuilderView::initializeGL()
{
	MySetPixelFormat(::GetDC(m_hWnd));

	// 获得绘图描述表
	hdc = ::GetDC(m_hWnd);
	// 创建渲染描述表
	hglrc = wglCreateContext(hdc);
	// 使绘图描述表为当前调用现程的当前绘图描述表
	wglMakeCurrent(hdc, hglrc);

	// 初始化Glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		AfxMessageBox("glewInit failed, something is seriously wrong.");
	}

	//glClearColor(0.5f, 0.6f, 0.8f, 1.0f);
	// Set sky color
	int cColor[3];
	getValueFromString(std::string(FLAGS_color_sky), cColor);
	glClearColor(cColor[0] / 255.0f, cColor[1] / 255.0f, cColor[2] / 255.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);// 开启遮挡

}

void CMy3DRebuilderView::renderScene()
{
	wglMakeCurrent(hdc, hglrc);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //清除颜色缓存和深度缓存

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	m_camera.look();

	renderPointsCloud();
	
	if (FLAGS_draw_box)
		box.DrawBox();

#if 1
	s += 0.005;
	if (s > 1.0)
		s = 0.1;
	step = step + 1.0;
	if (step > 360.0)
		step = step - 360.0;
	glPushMatrix();
	glScalef(s, s, s);
	glRotatef(step, 0.0, 1.0, 0.0);
	glRotatef(step, 0.0, 0.0, 1.0);
	glRotatef(step, 1.0, 0.0, 0.0);
	DrawColorBox();
	glPopMatrix();
	glFlush();
#endif
	SwapBuffers(hdc);

	if (m_bDenseFinish)
	{
		static int nPrintScreen = 0;
		static int nFrameCount = 0;
		std::string strPathBMP = FLAGS_output_image_directory;
		
		if (!theia::DirectoryExists(strPathBMP))
			theia::CreateNewDirectory(strPathBMP);

		if (0 == nFrameCount++ % n_fps && min_num_views_for_track >= -1)
		{
			min_num_views_for_track--;
			std::ostringstream os;
			os << strPathBMP << std::uppercase << std::setfill('0') << std::setw(2) << nPrintScreen++ << ".bmp";
			printScreen(os.str(), FLAGS_width, FLAGS_height);
		}

		if (min_num_views_for_track == -1 )
		{
			if (bOutputEnable)
			{
				compressBMP(FLAGS_format, nPrintScreen, FLAGS_output_image_directory,
					m_strPathExe, FLAGS_name, FLAGS_fps, FLAGS_width, FLAGS_height);
				bOutputEnable = false;
				nPrintScreen = 0;
			}
		}
	}
}

void CMy3DRebuilderView::renderPointsCloud()
{

	glPointSize( FLAGS_draw_point_size );
	glBegin(GL_POINTS);
	for (int i = 0; i < world_points.size(); i++) {
		if (num_views_for_track[i] < min_num_views_for_track) {
			continue;
		}
		Eigen::Vector3f color;
		if (FLAGS_same_color_point)
			color = Eigen::Vector3f(nColorPoint[0], nColorPoint[1], nColorPoint[2]) / 255.0;
		else
			color = point_colors[i] / 255.0;
		glColor3f( color[0], color[1], color[2] );

		glVertex3d(world_points[i].x(), world_points[i].y(), world_points[i].z());
	}
	glEnd();
}

void CMy3DRebuilderView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO:  在此处添加消息处理程序代码
	glViewport(0, 0, (GLsizei)(cx), (GLsizei)(cy));				// Reset The Current Viewport
	glMatrixMode(GL_PROJECTION);										// Select The Projection Matrix
	glLoadIdentity();													// Reset The Projection Matrix
	gluPerspective(45.0f, (GLfloat)(cx) / (GLfloat)(cy),			// Calculate The Aspect Ratio Of The Window
		1.0f, CLIP_FAR_DISTANCE);
	glMatrixMode(GL_MODELVIEW);										// Select The Modelview Matrix
	glLoadIdentity();

	FLAGS_width = cx;
	FLAGS_height = cy;
}

void CMy3DRebuilderView::DrawColorBox(void)
{
	GLfloat p1[] = { 0.5, -0.5, -0.5 }, p2[] = { 0.5, 0.5, -0.5 },
		p3[] = { 0.5, 0.5, 0.5 }, p4[] = { 0.5, -0.5, 0.5 },
		p5[] = { -0.5, -0.5, 0.5 }, p6[] = { -0.5, 0.5, 0.5 },
		p7[] = { -0.5, 0.5, -0.5 }, p8[] = { -0.5, -0.5, -0.5 };

	GLfloat m1[] = { 1.0, 0.0, 0.0 }, m2[] = { -1.0, 0.0, 0.0 },
		m3[] = { 0.0, 1.0, 0.0 }, m4[] = { 0.0, -1.0, 0.0 },
		m5[] = { 0.0, 0.0, 1.0 }, m6[] = { 0.0, 0.0, -1.0 };

	GLfloat c1[] = { 0.0, 0.0, 1.0 }, c2[] = { 0.0, 1.0, 1.0 },
		c3[] = { 1.0, 1.0, 1.0 }, c4[] = { 1.0, 0.0, 1.0 },
		c5[] = { 1.0, 0.0, 0.0 }, c6[] = { 1.0, 1.0, 0.0 },
		c7[] = { 0.0, 1.0, 0.0 }, c8[] = { 1.0, 1.0, 1.0 };

	glBegin(GL_QUADS); //绘制多个四边形

	glColor3fv(c1);
	glNormal3fv(m1);
	glVertex3fv(p1);
	glColor3fv(c2);
	glVertex3fv(p2);
	glColor3fv(c3);
	glVertex3fv(p3);
	glColor3fv(c4);
	glVertex3fv(p4);

	glColor3fv(c5);
	glNormal3fv(m5);
	glVertex3fv(p5);
	glColor3fv(c6);
	glVertex3fv(p6);
	glColor3fv(c7);
	glVertex3fv(p7);
	glColor3fv(c8);
	glVertex3fv(p8);

	glColor3fv(c5);
	glNormal3fv(m3);
	glVertex3fv(p5);
	glColor3fv(c6);
	glVertex3fv(p6);
	glColor3fv(c3);
	glVertex3fv(p3);
	glColor3fv(c4);
	glVertex3fv(p4);

	glColor3fv(c1);
	glNormal3fv(m4);
	glVertex3fv(p1);
	glColor3fv(c2);
	glVertex3fv(p2);
	glColor3fv(c7);
	glVertex3fv(p7);
	glColor3fv(c8);
	glVertex3fv(p8);

	glColor3fv(c2);
	glNormal3fv(m5);
	glVertex3fv(p2);
	glColor3fv(c3);
	glVertex3fv(p3);
	glColor3fv(c6);
	glVertex3fv(p6);
	glColor3fv(c7);
	glVertex3fv(p7);

	glColor3fv(c1);
	glNormal3fv(m6);
	glVertex3fv(p1);
	glColor3fv(c4);
	glVertex3fv(p4);
	glColor3fv(c5);
	glVertex3fv(p5);
	glColor3fv(c8);
	glVertex3fv(p8);

	glEnd();

}


BOOL CMy3DRebuilderView::OnEraseBkgnd(CDC* pDC)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值

	return TRUE;// CView::OnEraseBkgnd(pDC);
}


void CMy3DRebuilderView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	renderScene();

	fps_mm.check();

	outputInfo( fps_mm.fps(), "fps:", false );

	CView::OnTimer(nIDEvent);
}


void CMy3DRebuilderView::OnSelectImagePath()
{
	// TODO:  在此添加命令处理程序代码
	BROWSEINFO bi;
	char Buffer[MAX_PATH];

	//初始化入口参数bi开始
	bi.hwndOwner = NULL;
	bi.pidlRoot = NULL;//初始化制定的root目录很不容易
	bi.pszDisplayName = Buffer;//此参数如为NULL则不能显示对话框
	bi.lpszTitle = "选择图片路径";
	bi.ulFlags = BIF_EDITBOX;//带编辑框的风格
	bi.lpfn = NULL;
	bi.lParam = 0;
	bi.iImage = IDR_MAINFRAME;
	//初始化入口参数bi结束

	std::string strMessage = m_imagePath;
	strMessage = m_imagePath + "正在选择图片路径...";
	outputInfo(strMessage.c_str());
	LOG(INFO) << strMessage;

	LPITEMIDLIST pIDList = SHBrowseForFolder(&bi);//调用显示选择对话框

	if (pIDList)
	{
		SHGetPathFromIDList(pIDList, Buffer);

		//取得文件夹路径到Buffer里
		m_imagePath = std::string(Buffer) + "\\";

		outputInfo(m_imagePath.c_str());
		outputInfo("图片路径已选中");
		LOG(INFO) << m_imagePath << "图片路径已选中";
	}

	
	// free memory used   
	IMalloc * imalloc = 0;
	if (SUCCEEDED(SHGetMalloc(&imalloc)))
	{
		imalloc->Free(pIDList);
		imalloc->Release();
	}
}

#if 0
void CMy3DRebuilderView::AddImagesToReconstructionBuilderDIY(
	ReconstructionBuilder* reconstruction_builder) {
	std::vector<std::string> image_files;
	std::string strWildcard = m_imagePath + "*.jpg";

	outputInfo(strWildcard.c_str());
	if (false == theia::GetFilepathsFromWildcard(strWildcard, &image_files))
	{
		outputInfo("路径中无法找到图片文件！");
		return;
		//<< "Could not find images that matched the filepath: " << m_imagePath
		//<< ". NOTE that the ~ filepath is not supported.";
	}
	else
	{
		outputInfo("路径中已经找到图片文件！");
	}

	// Load calibration file if it is provided.
	std::unordered_map<std::string, theia::CameraIntrinsicsPrior>
		camera_intrinsics_prior;
	if (FLAGS_calibration_file.size() != 0) {
		CHECK(theia::ReadCalibration(FLAGS_calibration_file,
			&camera_intrinsics_prior))
			<< "Could not read calibration file.";
	}

	// Add images with possible calibration. When the intrinsics group id is
	// invalid, the reconstruction builder will assume that the view does not
	// share its intrinsics with any other views.
	theia::CameraIntrinsicsGroupId intrinsics_group_id =
		theia::kInvalidCameraIntrinsicsGroupId;
	if (FLAGS_shared_calibration) {
		intrinsics_group_id = 0;
	}

	for (const std::string& image_file : image_files) {
		std::string image_filename;
		CHECK(theia::GetFilenameFromFilepath(image_file, true, &image_filename));

		const theia::CameraIntrinsicsPrior* image_camera_intrinsics_prior =
			FindOrNull(camera_intrinsics_prior, image_filename);
		if (image_camera_intrinsics_prior != nullptr) {
			CHECK(reconstruction_builder->AddImageWithCameraIntrinsicsPrior(
				image_file, *image_camera_intrinsics_prior, intrinsics_group_id));
		}
		else {
			CHECK(reconstruction_builder->AddImage(image_file, intrinsics_group_id));
		}

		outputInfo(image_file.c_str());
		outputInfo("成功导入！");

	}

	// Add black and write image masks for any images if those are provided.
	// The white part of the mask indicates the area for the keypoints extraction.
	// The mask is a basic black and white image (jpg, png, tif etc.), where white
	// is 1.0 and black is 0.0. Its name must content the associated image's name
	// (e.g. 'image0001_mask.jpg' is the mask of 'image0001.png').
	std::vector<std::string> mask_files;
	if (FLAGS_image_masks.size() != 0) {
		CHECK(theia::GetFilepathsFromWildcard(FLAGS_image_masks, &mask_files))
			<< "Could not find image masks that matched the filepath: "
			<< FLAGS_image_masks
			<< ". NOTE that the ~ filepath is not supported.";
		if (mask_files.size() > 0) {
			for (const std::string& image_file : image_files) {
				std::string image_filename;
				CHECK(theia::GetFilenameFromFilepath(image_file,
					false,
					&image_filename));
				// Find and add the associated mask
				for (const std::string& mask_file : mask_files) {
					if (mask_file.find(image_filename) != std::string::npos) {
						CHECK(reconstruction_builder->AddMaskForFeaturesExtraction(
							image_file,
							mask_file));
						break;
					}
				}
			}
		}
		else {
			LOG(WARNING) << "No image masks found in: " << FLAGS_image_masks;
		}
	}

	// Extract and match features.
	if (reconstruction_builder->ExtractAndMatchFeatures())
	{
		std::string strMessage = FLAGS_matches_file;
		outputInfo(strMessage.c_str());
		outputInfo("匹配文件已保存，成功提取特征点并完成匹配");
	}
	else
	{
		std::string strMessage = FLAGS_matches_file;
		outputInfo(strMessage.c_str());
		outputInfo("匹配文件无法保存，特征提取或匹配过程出现异常");
	}
}


void CMy3DRebuilderView::build_reconstruction(std::vector<Reconstruction *>& reconstructions)
{
	const ReconstructionBuilderOptions options =
		SetReconstructionBuilderOptions();

	LOG(INFO) << formatStructure(options); 

	ReconstructionBuilder reconstruction_builder(options, m_strPathExe);
	// If matches are provided, load matches otherwise load images.
	if (FLAGS_matches_file.size() != 0) {
		AddMatchesToReconstructionBuilder(&reconstruction_builder);
	}
	else if (FLAGS_images.size() != 0) {
		AddImagesToReconstructionBuilder(&reconstruction_builder);
	}
	else {
		//LOG(FATAL)
			// You must specifiy either images to reconstruct or a match file.";
		outputInfo("未提供原始图片路径或中间匹配结果match文件");
	}

	if (false == reconstruction_builder.BuildReconstruction(&reconstructions))
		outputInfo("无法重构");//" Could not create a reconstruction.";


	if (reconstructions.size())
		reconstruction = reconstructions[0];
	else
		return ;

	LOG(INFO) << "开始为点云配置颜色：";
	theia::ColorizeReconstruction(m_imagePath.c_str(),
		FLAGS_num_threads,
		reconstruction);
	outputInfo("点云匹配颜色");
	LOG(INFO) << "为点云配置颜色完成！";


	theia::WriteReconstruction(*reconstruction,
		FLAGS_output_reconstruction);

	std::string resultPath(FLAGS_output_reconstruction);
	outputInfo(resultPath.c_str());
	outputInfo("点云文件成功保存，稀疏重建完成！");
}
#endif

void CMy3DRebuilderView::OnExecuteReconstructionSparse()
{
	// TODO:  在此添加命令处理程序代码

	if (m_imagePath.empty())
		m_imagePath = FLAGS_image_directory;

	if (!updateImagePath())
		return;

	outputInfo(m_imagePath.c_str());
	outputInfo("正在进行稀疏重建...");

	build_reconstruction(reconstruction, m_strPathExe);

	outputInfo("执行稀疏重建完成！");

	loadAndDisplaySparseResult();

}

#if 0
void CMy3DRebuilderView::CreateDirectoryIfDoesNotExist(const std::string& directory) {
	if (!theia::DirectoryExists(directory)) {
		CHECK(theia::CreateNewDirectory(directory))
			<< "Could not create the directory: " << directory;
	}
}

int CMy3DRebuilderView::WriteCamerasToPMVS(const theia::Reconstruction& reconstruction) {
	const std::string txt_dir = FLAGS_pmvs_working_directory + "/txt";
	CreateDirectoryIfDoesNotExist(txt_dir);
	const std::string visualize_dir = FLAGS_pmvs_working_directory + "/visualize";

	std::vector<std::string> image_files;
	std::string strWildcard = m_imagePath + "*.jpg";

	outputInfo(strWildcard.c_str());
	if (false == theia::GetFilepathsFromWildcard(strWildcard, &image_files))
	{
		outputInfo("路径中无法找到图片文件！");
		return -1;
	}
	else
	{
		outputInfo("路径中已经找到图片文件！");
	}

	// Format for printing eigen matrices.
	const Eigen::IOFormat unaligned(Eigen::StreamPrecision, Eigen::DontAlignCols);

	int current_image_index = 0;
	for (int i = 0; i < image_files.size(); i++) {
		std::string image_name;
		CHECK(theia::GetFilenameFromFilepath(image_files[i], true, &image_name));
		const theia::ViewId view_id = reconstruction.ViewIdFromName(image_name);
		if (view_id == theia::kInvalidViewId) {
			continue;
		}

		LOG(INFO) << "Undistorting image " << image_name;
		const theia::Camera& distorted_camera =
			reconstruction.View(view_id)->Camera();
		theia::Camera undistorted_camera;
		CHECK(theia::UndistortCamera(distorted_camera, &undistorted_camera));

		theia::FloatImage distorted_image(image_files[i]);
		theia::FloatImage undistorted_image;
		if( FLAGS_undistort )
		CHECK(theia::UndistortImage(distorted_camera,
			distorted_image,
			undistorted_camera,
			&undistorted_image));
		else
			undistorted_image = distorted_image;


		LOG(INFO) << "Exporting parameters for image: " << image_name;

		// Copy the image into a jpeg format with the filename in the form of
		// %08d.jpg.
		const std::string new_image_file = theia::StringPrintf(
			"%s/%08d.jpg", visualize_dir.c_str(), current_image_index);
		undistorted_image.Write(new_image_file);

		// Write the camera projection matrix.
		const std::string txt_file = theia::StringPrintf(
			"%s/%08d.txt", txt_dir.c_str(), current_image_index);
		theia::Matrix3x4d projection_matrix;
		undistorted_camera.GetProjectionMatrix(&projection_matrix);
		std::ofstream ofs(txt_file);
		ofs << "CONTOUR" << std::endl;
		ofs << projection_matrix.format(unaligned);
		ofs.close();

		++current_image_index;
	}

	return current_image_index;
}

void CMy3DRebuilderView::WritePMVSOptions(const std::string& working_dir,
	const int num_images) {
	std::ofstream ofs(working_dir + "/pmvs_options.txt");
	ofs << "level 1" << std::endl;
	ofs << "csize 2" << std::endl;
	ofs << "threshold 0.7" << std::endl;
	ofs << "wsize 7" << std::endl;
	ofs << "minImageNum 3" << std::endl;
	ofs << "CPU " << FLAGS_num_threads << std::endl;
	ofs << "setEdge 0" << std::endl;
	ofs << "useBound 0" << std::endl;
	ofs << "useVisData 0" << std::endl;
	ofs << "sequence -1" << std::endl;
	ofs << "timages -1 0 " << num_images << std::endl;
	ofs << "oimages 0" << std::endl;
}

void CMy3DRebuilderView::export_to_pmvs(theia::Reconstruction& reconstruction)
{
	outputInfo("正在预备稠密重建...");

	// Set up output directories.
	CreateDirectoryIfDoesNotExist(FLAGS_pmvs_working_directory);
	const std::string visualize_dir = FLAGS_pmvs_working_directory + "/visualize";
	CreateDirectoryIfDoesNotExist(visualize_dir);
	const std::string txt_dir = FLAGS_pmvs_working_directory + "/txt";
	CreateDirectoryIfDoesNotExist(txt_dir);
	const std::string models_dir = FLAGS_pmvs_working_directory + "/models";
	CreateDirectoryIfDoesNotExist(models_dir);

	const int num_cameras = WriteCamerasToPMVS(reconstruction);
	WritePMVSOptions(FLAGS_pmvs_working_directory, num_cameras);

	const std::string lists_file = FLAGS_pmvs_working_directory + "/list.txt";
	const std::string bundle_file =
		FLAGS_pmvs_working_directory + "/bundle.rd.out";
	if(theia::WriteBundlerFiles(reconstruction, lists_file, bundle_file))
		outputInfo(" 导出pmvs成功，稠密重建已经做好预备");
	else
		outputInfo(" 导出pmvs失败");

}
#endif

bool CMy3DRebuilderView::updateImagePath()
{
	if (m_imagePath.empty())
	{
		outputInfo("图片路径为空，请设置图片路径...");
		return false;
	}

	FLAGS_input_images = m_imagePath;
	FLAGS_images = FLAGS_input_images + "*.jpg";
	FLAGS_output_matches_file = FLAGS_input_images + "output.matches";
	FLAGS_output_reconstruction = FLAGS_input_images + "result";
	FLAGS_matching_working_directory = FLAGS_input_images + "features\\";
	FLAGS_pmvs_working_directory = FLAGS_input_images + "pmvs\\";
	FLAGS_ply_file = FLAGS_pmvs_working_directory + "models\\option-0000.ply";

	CreateDirectoryIfDoesNotExist(FLAGS_matching_working_directory);
	ReCreateDirectory(FLAGS_matching_working_directory);

	return true;
}
#if 0
void CMy3DRebuilderView::lanch_external_bin(String& bin, String& parameter, String& path, int nShowType)
{
	SHELLEXECUTEINFO ShExecInfo = { 0 };
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = bin.c_str();
	ShExecInfo.lpParameters = parameter.c_str();
	ShExecInfo.lpDirectory = path.c_str();
	ShExecInfo.nShow = nShowType;
	ShExecInfo.hInstApp = NULL;
	ShellExecuteEx(&ShExecInfo);

	long waitStatus = WaitForSingleObject(ShExecInfo.hProcess, INFINITE);

	if (waitStatus)
		printf("failed \n");
	else
		printf("succeed \n");

}

String CMy3DRebuilderView::getPath(String& strFullPath)
{
	int indexEnd = strFullPath.find_last_of('\\');
	return strFullPath.substr(0, indexEnd+1);
}

void CMy3DRebuilderView::run_pmvs(String &exePath)
{
	lanch_external_bin(String("cmvs.exe"), FLAGS_pmvs_working_directory, exePath);

	lanch_external_bin(String("genOption.exe"), FLAGS_pmvs_working_directory, exePath);

	String parameter = FLAGS_pmvs_working_directory + " option-0000";
	lanch_external_bin(String("pmvs2.exe"), parameter, exePath);

}
#endif
void CMy3DRebuilderView::OnExecuteReconstructionDense()
{
	if( !updateImagePath() )
		return;

	if (!FLAGS_build)
	{
		loadAndDisplayDenseResult();
		return;
	}

	// TODO:  在此添加命令处理程序代码
	LOG(INFO) << "正在进行稠密重建...";
	outputInfo("正在进行稠密重建...");

	if ( NULL == reconstruction )
	{
		reconstruction = new theia::Reconstruction();

		std::string strMessage = FLAGS_output_reconstruction;
		outputInfo(strMessage.c_str());

		if (false == ReadReconstruction(FLAGS_output_reconstruction, reconstruction))
			outputInfo(" 稀疏重建数据无法读取");
		else
			outputInfo(" 稀疏重建数据成功读取");

		// Centers the reconstruction based on the absolute deviation of 3D points.
		reconstruction->Normalize();
	}

	if (m_imagePath.empty())
		m_imagePath = FLAGS_image_directory;

#if 1
	outputInfo("正在预备稠密重建...");
	if (export_to_pmvs(*reconstruction, FLAGS_pmvs_working_directory, FLAGS_undistort))
		outputInfo(" 导出pmvs成功，稠密重建已经做好预备");
	else
		outputInfo(" 导出pmvs失败");
#endif

#if 1
	LOG(INFO) << "开始执行稠密重建：";
	run_pmvs(m_strPathExe.c_str(), FLAGS_pmvs_working_directory, FLAGS_threshold_group);
	LOG(INFO) << "执行稠密重建完成！";

	String resultPath = FLAGS_pmvs_working_directory + "option-0000";
	outputInfo(resultPath.c_str(), "");
	outputInfo("点云文件成功保存，稠密重建完成！");

#endif
	loadAndDisplayDenseResult();

}


void CMy3DRebuilderView::OnViewSparseResult()
{
	// TODO:  在此添加命令处理程序代码

	loadAndDisplaySparseResult();


}

void CMy3DRebuilderView::getEyePositionFromSparseResult(float fEyePosition[])
{
	Eigen::Vector3d cameraPosition = cameras[0].GetPosition();

	fEyePosition[0] = cameraPosition.x();
	fEyePosition[1] = cameraPosition.y();
	fEyePosition[2] = cameraPosition.z();

}
void CMy3DRebuilderView::updateEyePosition()
{
	Eigen::Vector3f& minPoint = box.getMinPoint();
	Eigen::Vector3f& maxPoint = box.getMaxPoint();

	Eigen::Vector3f midPoint = box.getMidPoint();
	Eigen::Vector3f sizeRect = box.getSizePoint();;

	double lengthMax = sizeRect.x() > sizeRect.z() ? sizeRect.x() : sizeRect.z();

	float fDistance[3];
	getValueFromString(std::string(FLAGS_distance), fDistance);

	float fEyePosition[3] = { 0, 0, 0 };

	// 场景竖直情况，采用相机视角
	if (sizeRect.y() > lengthMax)
		FLAGS_view_type = VIEW_CAMERA;

	switch (FLAGS_view_type)
	{
	case VIEW_PERSPECTIVE:
		fEyePosition[0] += midPoint.x() - sizeRect.x() * fDistance[0];
		fEyePosition[1] += midPoint.y() - sizeRect.y() * fDistance[1];
		fEyePosition[2] += midPoint.z() - sizeRect.z() * fDistance[2];
		break;

	case VIEW_TOP:
		fEyePosition[0] += midPoint.x();
		fEyePosition[1] += midPoint.y() + sizeRect.y()*fDistance[1];
		fEyePosition[2] += midPoint.z();
		break;

	case VIEW_FREE:
		fEyePosition[0] += midPoint.x();
		fEyePosition[1] += midPoint.y();
		fEyePosition[2] += midPoint.z() + lengthMax*fDistance[2];
		break;

	default:
		break;
	}

	if (VIEW_CAMERA == FLAGS_view_type)
		getEyePositionFromSparseResult(fEyePosition);

	m_camera.setEye(fEyePosition[0], fEyePosition[1], fEyePosition[2]);
	m_camera.setTarget(midPoint.x(), midPoint.y(), midPoint.z());
	m_camera.setDistance(sizeRect.z());
	m_camera.setViewType(FLAGS_view_type);

	m_camera.setSpeed(lengthMax*0.01);
}

void CMy3DRebuilderView::loadAndDisplaySparseResult()
{
	if (!updateImagePath())
		return;

	world_points.clear();
	point_normals.clear();
	point_colors.clear();

	outputInfo("正在显示稀疏重建结果...");
	LOG(INFO) << "正在显示稀疏重建结果...";

	if (NULL == reconstruction)
	{
		reconstruction = new theia::Reconstruction();

		FLAGS_input_images = m_imagePath;
		FLAGS_output_reconstruction = FLAGS_input_images + "result";
		outputInfo(FLAGS_output_reconstruction.c_str());

		if (false == ReadReconstruction(FLAGS_output_reconstruction, reconstruction))
			outputInfo(" 稀疏重建数据无法读取");
		else
			outputInfo(" 稀疏重建数据成功读取");

		// Centers the reconstruction based on the absolute deviation of 3D points.
		if (reconstruction->NumViews())
			reconstruction->Normalize();
	}

	// Set up camera drawing.
	cameras.reserve(reconstruction->NumViews());
	for (const theia::ViewId view_id : reconstruction->ViewIds()) {
		const auto* view = reconstruction->View(view_id);
		if (view == nullptr || !view->IsEstimated()) {
			continue;
		}
		cameras.emplace_back(view->Camera());
	}

	// Set up world points and colors.
	world_points.reserve(reconstruction->NumTracks());
	point_colors.reserve(reconstruction->NumTracks());
	for (const theia::TrackId track_id : reconstruction->TrackIds()) {
		const auto* track = reconstruction->Track(track_id);
		if (track == nullptr || !track->IsEstimated()) {
			continue;
		}
		world_points.emplace_back(track->Point().hnormalized());
		point_colors.emplace_back(track->Color().cast<float>());
		num_views_for_track.emplace_back(track->NumViews());
	}

	box.calculate(world_points);

	FLAGS_view_type = VIEW_CAMERA;

	updateEyePosition();

	min_num_views_for_track = -1;

	outputInfo("稀疏重建结果显示完成！");
	LOG(INFO) << "稀疏重建结果显示完成！";
}


void CMy3DRebuilderView::OnPrintScreen()
{
	// TODO:  在此添加命令处理程序代码
	printScreen("1.bmp", FLAGS_width, FLAGS_height);

	std::string msg = m_strPathExe + "1.bmp已保存，截屏完成！";
	outputInfo( msg.c_str() );

}

void CMy3DRebuilderView::printScreen(std::string filename, int width, int height)
{
	int nLineByte = (width*3 + 3) / 4 * 4;	// 一行字节数必须被4整除
	char* buf = new char[nLineByte * height];
	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
	glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, buf);

	saveBMPFile(filename, width, height, buf);

	delete[] buf;
}


void CMy3DRebuilderView::OnRebuildOneKey()
{
	// TODO:  在此添加命令处理程序代码
	CSettingRebuildShowMode dlg;
	dlg.DoModal();

	lanch_external_bin(String("build_view_reconstruction.exe"),
String("--input_images=E:\\3d\\2017_03\\ --output_images=E:\\3d\\output\\ \
--same_color=0 --color_point=(0,255,0) --color_sky=(0,0,0) \
--point_size=3 --eye_position=(2,-10,-3) --eye_angle=(90,30,0) \
--flagfile=E:\\3d\\exe\\build_reconstruction_flags.txt \
--view=0 --build=0"),
String("E:\\3d\\exe\\"));

}


void CMy3DRebuilderView::OnOpenUserManual()
{
	// TODO:  在此添加命令处理程序代码

	std::string filename = m_strPathExe + "@参数说明.txt";
	lanch_external_bin(String("notepad.exe"), filename, String(""), SW_SHOW);
}

void CMy3DRebuilderView::setColorBG(float r, float g, float b)
{
	glClearColor(r, g, b, 1.0);
}

void CMy3DRebuilderView::setColorPoint(float r, float g, float b)
{
	nColorPoint[0] = r * 255;
	nColorPoint[1] = g * 255; 
	nColorPoint[2] = b * 255;
}

void CMy3DRebuilderView::setColorPointFlag(bool bFlag)
{
	FLAGS_same_color_point = bFlag;
}

void CMy3DRebuilderView::setSizePoint(int nSize)
{
	FLAGS_draw_point_size = nSize;
}

void CMy3DRebuilderView::setBuildFlag(bool bFlag)
{
	FLAGS_build = bFlag;
}

void CMy3DRebuilderView::setOutputFormat(std::string format)
{
	FLAGS_format = format;
}

void CMy3DRebuilderView::setOutputFPS(int nSize)
{
	FLAGS_fps = nSize;
}

void CMy3DRebuilderView::setOutputName(std::string name)
{
	FLAGS_name = name;
}

void CMy3DRebuilderView::setOutputLength(int nSize)
{
	FLAGS_length = nSize;
}
