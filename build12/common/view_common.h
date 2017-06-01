// Copyright (C) 2014 The Regents of the University of California (Regents).
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//
//     * Neither the name of The Regents or University of California nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Please contact the author of this library if you have any questions.
// Author: Chris Sweeney (cmsweeney@cs.ucsb.edu)

#include <Eigen/Core>
#include <glog/logging.h>
#include <gflags/gflags.h>
#include <theia/theia.h>
#include <string>
#include <vector>

#include <windows.h>
#include <glew.h>
#include <glut.h>

#include "bmpHeader.h"
#include "WriteGIF.h"
#include "simpleBMP.h"
#include <OpenImageIO/imagebuf.h>
#include <stlplus3/file_system.hpp>
#include <fstream>

#include "BoundingBox.h"

#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/types_c.h>  // include it before #include <OpenImageIO/imagebufalgo.h>
#include <opencv2/highgui/highgui_c.h>

#include "LaunchPMVS2.h"

#define		PI		3.1415926	

DEFINE_string(reconstruction, "", "Reconstruction file to be viewed.");
DEFINE_bool(same_color, false, "bool on/off to use same color for point. eg:0 ");
DEFINE_int32(point_size, 1, "bool on/off to use same color for point. eg:0 ");
DEFINE_string(color_sky, "(128,150,200)", "color of sky. eg:(128,150,200)blue ");
DEFINE_string(color_point, "(255,255,255)", "color of point. eg:(255,255,255)white ");

DEFINE_string(output_images, "./output/", "output image directory");
DEFINE_string(format, "jpg+gif+avi+mp4", "jpg, gif, avi, mp4 ");
DEFINE_bool(y_flip, false, "y direction 1-up,  -1-down");
DEFINE_bool(view, false, "bool on/off to view. eg:0 ");
DEFINE_int32(output_speed, 1000, "output speed 1-1000");
DEFINE_int32(width, 1280, "window width");
DEFINE_int32(height, 1024, "window height");
DEFINE_string(distance, "(0.1,0.6,0.2)", "set distance of view");
DEFINE_bool(draw_box, false, "draw bounding box");
DEFINE_bool(exit_fast, true, "exit when output finish");
DEFINE_bool(swap_yz, false, "swap y and z");
DEFINE_bool(head_flip, true, "head flip");
DEFINE_bool(save_camera, true, "save camera property to file");
DEFINE_int32(view_type, 0, "0-perspective, 1-camera, 2-top, 3-free, 4-common");
DEFINE_string(eye_position, "(0,0,0)", "position of eye.");
DEFINE_string(eye_angle, "(0,0,0)", "angle of eye.");
DEFINE_bool(view_sparse, false, "view sparse or not");
DEFINE_bool(light, false, "turn on/off light");
DEFINE_double(fps, 2, "frame per second");
DEFINE_string(name, "0", "gif or mp4 file name");
DEFINE_double(length, 5, "length of vedio, unit: seconds");

std::string FLAGS_pmvs_working_directory;

Eigen::Vector2i window_position(200, 100);

int		nColorPoint[3];

Eigen::Vector2i window_size(1280, 1024);

// Containers for the data.
std::vector<theia::Camera> cameras;
theia::Vector3dVec world_points;
theia::Vector3fVec point_colors;
theia::Vector3fVec point_normals;
std::vector<int> num_views_for_track;

BoundingBox box;

// Parameters for OpenGL.
int width = 1200;
int height = 800;

// OpenGL camera parameters.
Eigen::Vector3f eye_position;
Eigen::Vector3f eye_position_default;
float zoom_default;// = -500.0;
float zoom;// = -500.0;
float delta_zoom = 1.1;
float speed = 0.1;
float speed_angle = 0.5;

// Rotation values for the navigation
Eigen::Vector2f navigation_rotation_default;// (45.0, 0.0);
Eigen::Vector2f navigation_rotation;// (45.0, 0.0);

// Position of the mouse when pressed
int mouse_pressed_x = 0, mouse_pressed_y = 0;
float last_x_offset = 0.0, last_y_offset = 0.0;
// Mouse button states
int left_mouse_button_active = 0, right_mouse_button_active = 0;

// Visualization parameters.
bool draw_cameras = false;
bool draw_axes = false;
bool draw_box = false;
float point_size = 1.0;
float normalized_focal_length = 1.0;
int min_num_views_for_track = FLAGS_fps * FLAGS_length;
double anti_aliasing_blend = 0.3;
std::ofstream fileCameraOut;
std::ifstream fileCameraIn;

//int y_up_direction = -1;// 1-up,  -1-down 
//extern int n_fps; // frame per second

extern int		nColorPoint[];
std::string strPathExe;

int		nImageCountOutput=0;

bool	bDenseFinish = false;
bool	bOutputFinish = false;

void	viewDenseResult();
void	convertSparseToDense();

void GetPerspectiveParams(double* aspect_ratio, double* fovy) {
  double focal_length = 800.0;
  *aspect_ratio = static_cast<double>(width) / static_cast<double>(height);
  *fovy = 2 * atan(height / (2.0 * focal_length)) * 180.0 / M_PI;
}

void ChangeSize(int w, int h) {
  // Prevent a divide by zero, when window is too short
  // (you cant make a window of zero width).
  if (h == 0) h = 1;

  width = w;
  height = h;

  // Use the Projection Matrix
  glMatrixMode(GL_PROJECTION);

  // Reset Matrix
  glLoadIdentity();

  // Set the viewport to be the entire window
  double aspect_ratio, fovy;
  GetPerspectiveParams(&aspect_ratio, &fovy);
  glViewport(0, 0, w, h);

  // Set the correct perspective.
  gluPerspective(fovy, aspect_ratio, 0.001f, 100000.0f);

  // Get Back to the Reconstructionview
  glMatrixMode(GL_MODELVIEW);
}

void DrawAxes(float length) {
  glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDisable(GL_LIGHTING);
  glLineWidth(5.0);
  glBegin(GL_LINES);
  glColor3f(1.0, 0.0, 0.0);
  glVertex3f(0, 0, 0);
  glVertex3f(length, 0, 0);

  glColor3f(0.0, 1.0, 0.0);
  glVertex3f(0, 0, 0);
  glVertex3f(0, length, 0);

  glColor3f(0.0, 0.0, 1.0);
  glVertex3f(0, 0, 0);
  glVertex3f(0, 0, length);
  glEnd();

  glPopAttrib();
  glLineWidth(1.0);
}

void DrawCamera(const theia::Camera& camera) {
  glPushMatrix();
  Eigen::Matrix4d transformation_matrix = Eigen::Matrix4d::Zero();
  transformation_matrix.block<3, 3>(0, 0) =
      camera.GetOrientationAsRotationMatrix().transpose();
  transformation_matrix.col(3).head<3>() = camera.GetPosition();
  transformation_matrix(3, 3) = 1.0;

  // Apply world pose transformation.
  glMultMatrixd(reinterpret_cast<GLdouble*>(transformation_matrix.data()));

  // Draw Cameras.
  glColor3f(1.0, 0.0, 0.0);

  // Create the camera wireframe. If intrinsic parameters are not set then use
  // the focal length as a guess.
  const float image_width =
      (camera.ImageWidth() == 0) ? camera.FocalLength() : camera.ImageWidth();
  const float image_height =
      (camera.ImageHeight() == 0) ? camera.FocalLength() : camera.ImageHeight();
  const float normalized_width = (image_width / 2.0) / camera.FocalLength();
  const float normalized_height = (image_height / 2.0) / camera.FocalLength();

  const Eigen::Vector3f top_left =
      normalized_focal_length *
      Eigen::Vector3f(-normalized_width, -normalized_height, 1);
  const Eigen::Vector3f top_right =
      normalized_focal_length *
      Eigen::Vector3f(normalized_width, -normalized_height, 1);
  const Eigen::Vector3f bottom_right =
      normalized_focal_length *
      Eigen::Vector3f(normalized_width, normalized_height, 1);
  const Eigen::Vector3f bottom_left =
      normalized_focal_length *
      Eigen::Vector3f(-normalized_width, normalized_height, 1);

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(top_right[0], top_right[1], top_right[2]);
  glVertex3f(top_left[0], top_left[1], top_left[2]);
  glVertex3f(bottom_left[0], bottom_left[1], bottom_left[2]);
  glVertex3f(bottom_right[0], bottom_right[1], bottom_right[2]);
  glVertex3f(top_right[0], top_right[1], top_right[2]);
  glEnd();
  glPopMatrix();
}


void DrawBox(float xMin, float yMin, float zMin, 
	float xMax, float yMax, float zMax)
{
	GLfloat p1[] = { xMax, yMin, zMin }, p2[] = { xMax, yMax, zMin },
		p3[] = { xMax, yMax, zMax }, p4[] = { xMax, yMin, zMax },
		p5[] = { xMin, yMin, zMax }, p6[] = { xMin, yMax, zMax },
		p7[] = { xMin, yMax, zMin }, p8[] = { xMin, yMin, zMin };


#if 0
	   6_______ 3	
	  /		  /|
	 /		 / |
	7------2   |
	|	   |   |
	|   5--|   4	
	|	   |  /
	|	   | /
	8------1
#endif

	glBegin(GL_QUADS); //绘制多个四边形

	glVertex3fv(p1);
	glVertex3fv(p2);
	glVertex3fv(p3);
	glVertex3fv(p4);

	glVertex3fv(p5);
	glVertex3fv(p6);
	glVertex3fv(p7);
	glVertex3fv(p8);

	glVertex3fv(p5);
	glVertex3fv(p6);
	glVertex3fv(p3);
	glVertex3fv(p4);

	glVertex3fv(p1);
	glVertex3fv(p2);
	glVertex3fv(p7);
	glVertex3fv(p8);

	glVertex3fv(p2);
	glVertex3fv(p3);
	glVertex3fv(p6);
	glVertex3fv(p7);

	glVertex3fv(p1);
	glVertex3fv(p4);
	glVertex3fv(p5);
	glVertex3fv(p8);

	glEnd();

}

void DrawPoints(const float point_scale,
                const float color_scale,
                const float alpha_scale) {
  const float default_point_size = point_size;
  const float default_alpha_scale = anti_aliasing_blend;

  // TODO(cmsweeney): Render points with the actual 3D point color! This would
  // require Theia to save the colors during feature extraction.
  //const Eigen::Vector3f default_color(0.05, 0.05, 0.05);

  // Enable anti-aliasing for round points and alpha blending that helps make
  // points look nicer.
  //glDisable(GL_LIGHTING);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_BLEND);
  glEnable(GL_POINT_SMOOTH);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // The coordinates for calculating point attenuation. This allows for points
  // to get smaller as the OpenGL camera moves farther away.
  GLfloat point_size_coords[3];
  point_size_coords[0] = 1.0f;
  point_size_coords[1] = 0.055f;
  point_size_coords[2] = 0.0f;
  glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, point_size_coords);


  glPointSize(point_scale * default_point_size);
  glBegin(GL_POINTS);
  for (int i = 0; i < world_points.size(); i++) {
    if (num_views_for_track[i] < min_num_views_for_track) {
      continue;
    }
	Eigen::Vector3f color;
	if (FLAGS_same_color)
		color = Eigen::Vector3f(nColorPoint[0], nColorPoint[1], nColorPoint[2]) / 255.0;
	else
		color = point_colors[i] / 255.0;
	glColor3f(color_scale * color[0],
		color_scale * color[1],
		color_scale * color[2]);
             // , alpha_scale * default_alpha_scale);
	glNormal3d(point_normals[i].x(), point_normals[i].y(), point_normals[i].z() );
	glVertex3d(world_points[i].x(), world_points[i].y(), world_points[i].z());
  }
  glEnd();

	// draw box
  if (FLAGS_draw_box && draw_box)
	{
		box.DrawBox();
	}

}

void printScreen(std::string filename, int width = 1024, int height = 768)
{
	char* buf = new char[3 * width * height];
	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
	glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, buf);

	saveBMPFile(filename, width, height, buf);

	delete[] buf;
}

void compressBMP()
{
	if (std::string::npos != FLAGS_format.find("jpg"))
	{
		for (int i = 0; i < nImageCountOutput; i++)
		{
			std::ostringstream osIn;
			osIn << FLAGS_output_images << std::uppercase << std::setfill('0') << std::setw(2) << i << ".bmp";
			OpenImageIO::ImageBuf image(osIn.str());

			std::ostringstream osOut;
			osOut << FLAGS_output_images << std::uppercase << std::setfill('0') << std::setw(2) << i << ".jpg";
			image.write(osOut.str(), "jpg");

			image.clear();
		}
	}

	if (std::string::npos != FLAGS_format.find("gif"))
	{
		gif::GIF* g = gif::newGIF(1.0/FLAGS_fps * 100); // unit: ten millisecond
		ClImgBMP	bmp;
		std::string strPathGIF(FLAGS_output_images);
		strPathGIF += FLAGS_name + ".gif";

		for (int i = 0; i < nImageCountOutput; i++)
		{
			std::ostringstream osIn;
			osIn << FLAGS_output_images << std::uppercase << std::setfill('0') << std::setw(2) << i << ".bmp";
			
			bmp.LoadImage(osIn.str().c_str());
			gif::addFrame(g, bmp.bmpInfoHeaderData.biWidth, bmp.bmpInfoHeaderData.biHeight, bmp.imgData, 0);
		}

		gif::write(g, strPathGIF.c_str());

		gif::dispose(g);	g = NULL;
	}


	if (std::string::npos != FLAGS_format.find("avi") || std::string::npos != FLAGS_format.find("mp4"))
	{
		std::string strPathAVI(FLAGS_output_images);
		strPathAVI += FLAGS_name + ".avi";

		CvSize size = cvSize(FLAGS_width, FLAGS_height);
		CvVideoWriter* writer = cvCreateVideoWriter(
			strPathAVI.c_str(), CV_FOURCC('D', 'I', 'V', 'X'), FLAGS_fps, size);

		if (writer)		{

			for (int i = 0; i < nImageCountOutput; i++)
			{
				std::ostringstream osIn;
				osIn << FLAGS_output_images << std::uppercase << std::setfill('0') << std::setw(2) << i << ".bmp";

				IplImage* iplImgOut = cvLoadImage(osIn.str().c_str());

				cvWriteToAVI(writer, iplImgOut);
			}

			cvReleaseVideoWriter(&writer);

			LOG(INFO) << strPathAVI << " 视频文件成功创建！";
		}
		else
			LOG(INFO) << strPathAVI << " 视频文件无法创建！";

	}

	if (std::string::npos != FLAGS_format.find("mp4"))
	{
		std::string exePath = getPath(std::string(strPathExe));

		std::string strPathAVI(FLAGS_output_images);
		strPathAVI += FLAGS_name + ".avi";

		std::string strPathMP4(FLAGS_output_images);
		strPathMP4 += FLAGS_name + ".mp4";

		std::string parameter("-i ");
		parameter += strPathAVI;
		parameter += " ";
		parameter += strPathMP4;

		lanch_external_bin(std::string("ffmpeg.exe"), parameter, exePath);

		if (std::string::npos == FLAGS_format.find("avi"))
		{
			bool breturn = stlplus::file_delete(strPathAVI);
			if (!breturn)
			{
				LOG(INFO) << strPathAVI << "视频文件未找到或无法删除！";
			}
		}
	}

	// delete bmp
	for (int i = 0; i < nImageCountOutput; i++)
	{
		std::ostringstream osIn;
		osIn << FLAGS_output_images << std::uppercase << std::setfill('0') << std::setw(2) << i << ".bmp";
		bool breturn = stlplus::file_delete(osIn.str());
		if (!breturn)
		{
			std::cout << osIn.str() << "图片文件未找到或无法删除！";
		}

	}

}

void RenderScene() {

	if (!FLAGS_view) // 第一帧
		glutHideWindow();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  Eigen::Vector3f& maxPoint = box.getMaxPoint();

  Eigen::Vector3f& midPoint = box.getMidPoint();
  Eigen::Vector3f& sizePoint = box.getSizePoint();

  float g_dir[3];
  float head_dir = 1;
  switch ( FLAGS_view_type )
  {
  case VIEW_PERSPECTIVE:

	  glTranslatef(0.0f, 0.0f, -0.5 * sizePoint.z() );

	  gluLookAt(eye_position.x(), eye_position.y(), eye_position.z(),
		  midPoint.x(), midPoint.y(), midPoint.z(),
		  0.0, -1.0, 0.0);
	  break;

  case VIEW_TOP:

	  gluLookAt(eye_position.x(), eye_position.y(), eye_position.z(),
		  midPoint.x(), midPoint.y(), midPoint.z(),
		  0.0, 0.0, -1.0);
	  break;

  case VIEW_CAMERA:

	  glTranslatef(0.0f, 0.0f, -4.0*0.034 / abs(eye_position.z() / maxPoint.z()));

	  gluLookAt(eye_position.x(), eye_position.y(), eye_position.z(),
		  midPoint.x(), midPoint.y(), midPoint.z(),
		  0.0, -1.0, 0.0);

	  break;

  case VIEW_FREE:

	  g_dir[0] = -sin(PI*navigation_rotation[0] / 180.0f);
	  g_dir[2] = -cos(PI*navigation_rotation[0] / 180.0f);
	  g_dir[1] = -sin(PI*navigation_rotation[1] / 180.0f);
	  if (FLAGS_head_flip)
		  head_dir = -1;

	  gluLookAt(
		  eye_position[0], eye_position[1], eye_position[2],
		  eye_position[0] + g_dir[0], eye_position[1] + g_dir[1], eye_position[2] + g_dir[2],
		  0, head_dir, 0);

	  break;

  case VIEW_COMMON:

	  // Transformation to the viewer origin.
	  glTranslatef(0.0, 0.0, zoom);
	  glRotatef(navigation_rotation[0], 1.0f, 0.0f, 0.0f);
	  glRotatef(navigation_rotation[1], 0.0f, 1.0f, 0.0f);
	  if (draw_axes) {
		  DrawAxes(10.0);
	  }

	  // Transformation from the viewer origin to the reconstruction origin.
	  glTranslatef(eye_position[0], eye_position[1], eye_position[2]);

	  break;

  default:
	  break;
  }

  // Each 3D point is rendered 3 times with different point sizes, color
  // intensity, and alpha blending. This allows for a more complete texture-like
  // rendering of the 3D points. These values were found to experimentally
  // produce nice visualizations on most scenes.
  const float small_point_scale = 1.0, medium_point_scale = 5.0,
              large_point_scale = 10.0;
  const float small_color_scale = 1.0, medium_color_scale = 1.2,
              large_color_scale = 1.5;
  const float small_alpha_scale = 1.0, medium_alpha_scale = 2.1,
              large_alpha_scale = 3.3;

  DrawPoints(small_point_scale, small_color_scale, small_alpha_scale);
  //DrawPoints(medium_point_scale, medium_color_scale, medium_alpha_scale);
  //DrawPoints(large_point_scale, large_color_scale, large_alpha_scale);

  // Draw the cameras.
  if (draw_cameras) {
    for (int i = 0; i < cameras.size(); i++) {
      DrawCamera(cameras[i]);
    }
  }

 
	  static int nPrintScreen = 0;
	  static int nFrameCount = 0;
	  std::string strPathBMP = FLAGS_output_images;

	  if (!theia::DirectoryExists(strPathBMP))
		  theia::CreateNewDirectory(strPathBMP);

	  int nFrameInterval = 1000.0f / FLAGS_output_speed + 1;
	  if (0 == ++nFrameCount % nFrameInterval && min_num_views_for_track >= -1)
	  {
		  //std::cout << "output: " << min_num_views_for_track  << "at time: " << clock() << std::endl;
		  min_num_views_for_track--;
		  std::ostringstream os;
		  os << strPathBMP << std::uppercase << std::setfill('0') << std::setw(2) << nPrintScreen++ << ".bmp";
		  printScreen(os.str(), window_size[0], window_size[1]);
		  nImageCountOutput++;
	  }


	if (min_num_views_for_track == -1)
	{
		if (!bDenseFinish && FLAGS_view_sparse)
		{
			convertSparseToDense();
			min_num_views_for_track = FLAGS_fps * FLAGS_length;
			viewDenseResult();
			bDenseFinish = true;
			return;
		}

		if (!bOutputFinish)
		{
			compressBMP();
			bOutputFinish = true;
		}
		if (FLAGS_exit_fast)
			exit(0);
	}
#if 0
	  if (min_num_views_for_track == -1 && FLAGS_output_image_type == 1)
	  {
		  bmp2gif	b2g(100);
		  std::string strPathGIF = strPathBMP + "1.GIF";
		  b2g.run(strPathBMP.c_str(), strPathGIF.c_str(), 10);
	  }
#endif

	  glutSwapBuffers();

}

void setEyeParameter(float pos[], float angle[])
{
	navigation_rotation_default[0] = angle[0];
	navigation_rotation_default[1] = angle[1];
	navigation_rotation = navigation_rotation_default;

	eye_position_default[0] = pos[0];
	eye_position_default[1] = pos[1];
	eye_position_default[2] = pos[2];
	eye_position = eye_position_default;
}

void MouseButton(int button, int state, int x, int y) {
  // get the mouse buttons
  if (button == GLUT_RIGHT_BUTTON) {
    if (state == GLUT_DOWN) {
      right_mouse_button_active += 1;
    } else {
      right_mouse_button_active -= 1;
    }
  } else if (button == GLUT_LEFT_BUTTON) {
    if (state == GLUT_DOWN) {
      left_mouse_button_active += 1;
      last_x_offset = 0.0;
      last_y_offset = 0.0;
    } else {
      left_mouse_button_active -= 1;
    }
  }

  // scroll event - wheel reports as button 3 (scroll up) and button 4 (scroll
  // down)
  if ((button == 3) || (button == 4)) {
    // Each wheel event reports like a button click, GLUT_DOWN then GLUT_UP
    if (state == GLUT_UP) return;  // Disregard redundant GLUT_UP events
    if (button == 3) {
      zoom *= delta_zoom;
    } else {
      zoom /= delta_zoom;
    }
  }

  mouse_pressed_x = x;
  mouse_pressed_y = y;
}

void MouseMove(int x, int y) {
  float x_offset = 0.0, y_offset = 0.0;

  // Rotation controls
  if (right_mouse_button_active) {
    navigation_rotation[0] += ((mouse_pressed_y - y) * 180.0f) / 200.0f;
    navigation_rotation[1] += ((mouse_pressed_x - x) * 180.0f) / 200.0f;

    mouse_pressed_y = y;
    mouse_pressed_x = x;

  } else if (left_mouse_button_active) {
    float delta_x = 0, delta_y = 0;
    const Eigen::AngleAxisf rotation(
        Eigen::AngleAxisf(theia::DegToRad(navigation_rotation[0]),
                          Eigen::Vector3f::UnitX()) *
        Eigen::AngleAxisf(theia::DegToRad(navigation_rotation[1]),
                          Eigen::Vector3f::UnitY()));

    // Panning controls.
    x_offset = (mouse_pressed_x - x);
    if (last_x_offset != 0.0) {
      delta_x = -(x_offset - last_x_offset) / 8.0;
    }
    last_x_offset = x_offset;

    y_offset = (mouse_pressed_y - y);
    if (last_y_offset != 0.0) {
      delta_y = (y_offset - last_y_offset) / 8.0;
    }
    last_y_offset = y_offset;

    // Compute the new viewer origin origin.
    eye_position +=
        rotation.inverse() * Eigen::Vector3f(delta_x, delta_y, 0);
  }
}

void processSpecialKeys(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_HOME:	// y
		eye_position.y() += speed;
		break;
	case GLUT_KEY_END:	// -y
		eye_position.y() -= speed;
		break;
	case GLUT_KEY_LEFT:	// - angle around x axis
		navigation_rotation.x() += speed_angle;
		break;
	case GLUT_KEY_RIGHT:	// - angle around x axis
		navigation_rotation.x() -= speed_angle;
		break;
	case GLUT_KEY_UP:	// - angle around y axis
		navigation_rotation.y() += speed_angle;
		break;
	case GLUT_KEY_DOWN:	// - angle around y axis
		navigation_rotation.y() -= speed_angle;
		break;
	}
}
void Keyboard(unsigned char key, int x, int y) {
  switch (key) {
    case 'r':  // reset viewpoint
      eye_position.setZero();
	  zoom = zoom_default;
      navigation_rotation.setZero();
      mouse_pressed_x = 0;
      mouse_pressed_y = 0;
      last_x_offset = 0.0;
      last_y_offset = 0.0;
      left_mouse_button_active = 0;
      right_mouse_button_active = 0;
	  point_size = FLAGS_point_size;
	  min_num_views_for_track = FLAGS_fps * FLAGS_length;
	  navigation_rotation = navigation_rotation_default;
	  eye_position = eye_position_default;
	  break;
	case 'o':	// output jpg or  gif/mp4
	  min_num_views_for_track = FLAGS_fps * FLAGS_length;

	  if (FLAGS_save_camera)
	  {
		  std::string filename = FLAGS_input_images + "camera.txt";
		  fileCameraOut.open(filename);
		  fileCameraOut << eye_position << std::endl
			   << navigation_rotation;
		  fileCameraOut.close();
	  }
	  break;
	case 'w':	// z
		eye_position.z() += speed;
		break;
	case 's':	// -z
		eye_position.z() -= speed;
		break;
	case 'd':	// x
		eye_position.x() += speed;
		break;
	case 'a':	// -x
		eye_position.x() -= speed;
		break;
    case 'z':
      zoom *= delta_zoom;
      break;
    case 'Z':
      zoom /= delta_zoom;
      break;
    case 'p':
      point_size /= 1.2;
      break;
    case 'P':
      point_size *= 1.2;
      break;
    case 'f':
      normalized_focal_length /= 1.2;
      break;
    case 'F':
      normalized_focal_length *= 1.2;
      break;
    case 'c':
      draw_cameras = !draw_cameras;
      break;
/*    case 'a':
      draw_axes = !draw_axes;
      break;*/
    case 't':
      ++min_num_views_for_track;
      break;
    case 'T':
      --min_num_views_for_track;
      break;
    case 'b':
		draw_box = !draw_box;
      if (anti_aliasing_blend > 0) {
        anti_aliasing_blend -= 0.01;
      }
      break;
    case 'B':
      if (anti_aliasing_blend < 1.0) {
        anti_aliasing_blend += 0.01;
      }
      break;
  }
}


template<typename T>
void getValueFromString(std::string str, T * cColor)
{
	std::istringstream in(str);
	char tmp;
	in >> tmp >> cColor[0] >> tmp >> cColor[1] >> tmp >> cColor[2];
}

void getEyePositionFromSparseResult(float fEyePosition[])
{
	Eigen::Vector3d cameraPosition = cameras[0].GetPosition();
	
	fEyePosition[0] = cameraPosition.x();
	fEyePosition[1] = cameraPosition.y();
	fEyePosition[2] = cameraPosition.z();

}

void setDefaultCameraProperty()
{

	float fEyePosition[3];
	getValueFromString(std::string(FLAGS_eye_position), fEyePosition);
	float fEyeAngle[3];
	getValueFromString(std::string(FLAGS_eye_angle), fEyeAngle);

	Eigen::Vector3f& midPoint = box.getMidPoint();
	Eigen::Vector3f& sizeRect = box.getSizePoint();

	double lengthMax = sizeRect.x() > sizeRect.z() ? sizeRect.x() : sizeRect.z();

	float fDistance[3];
	getValueFromString(std::string(FLAGS_distance), fDistance);
#if 1
	// 场景竖直情况，采用相机视角
	if (sizeRect.y() > lengthMax)
		FLAGS_view_type = VIEW_CAMERA;
#endif
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

	if (FLAGS_save_camera)
	{
		std::string filename = FLAGS_input_images + "camera.txt";
		fileCameraIn.open(filename);
		if (fileCameraIn.is_open())
		{
			fileCameraIn >> fEyePosition[0] >> fEyePosition[1] >> fEyePosition[2];
			fileCameraIn >> fEyeAngle[0] >> fEyeAngle[1] >> fEyeAngle[2];
		}
		fileCameraIn.close();
	}

	if (VIEW_CAMERA == FLAGS_view_type)
		getEyePositionFromSparseResult(fEyePosition);

	setEyeParameter(fEyePosition, fEyeAngle);

	speed = lengthMax * 0.01;

	if (FLAGS_head_flip)
	{
		speed *= -1;
		speed_angle *= -1;
	}
}

void getInt3FromString(std::string str, int * cColor)
{
	std::istringstream in(str);
	char tmp;
	in >> tmp >> cColor[0] >> tmp >> cColor[1] >> tmp >> cColor[2];
}

void gl_draw_points(int argc, char** argv)
{
	// Set up opengl and glut.
	glutInit(&argc, argv);
	glutInitWindowPosition(window_position[0], window_position[1]);
	window_size[0] = FLAGS_width;
	window_size[1] = FLAGS_height;
	glutInitWindowSize(window_size[0], window_size[1]);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("SDIOI Reconstruction Viewer");

#ifdef _WIN32
	// Set up glew.
	CHECK_EQ(GLEW_OK, glewInit())
		<< "Failed initializing GLEW.";
#endif

	// light 
	if (FLAGS_light)
		glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);								// Enable Light One
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Set the camera
	gluLookAt(0.0f, 0.0f, -6.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Set sky color
	int cColor[3];
	getInt3FromString(std::string(FLAGS_color_sky), cColor);

	glClearColor(cColor[0] / 255.0f, cColor[1] / 255.0f, cColor[2] / 255.0f, 1.0f);

	// Set point color / size
	getInt3FromString(std::string(FLAGS_color_point), nColorPoint);
	point_size = FLAGS_point_size;

	// register callbacks
	glutDisplayFunc(RenderScene);
	glutReshapeFunc(ChangeSize);
	glutMouseFunc(MouseButton);
	glutMotionFunc(MouseMove);
	glutKeyboardFunc(Keyboard);
	glutIdleFunc(RenderScene);
	glutSpecialFunc(processSpecialKeys);

	// enter GLUT event processing loop
	glutMainLoop();
}


void prepare_points_to_draw(theia::Reconstruction *reconstruction)
{
	if (reconstruction->NumViews())
	{
		// Centers the reconstruction based on the absolute deviation of 3D points.
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
		point_normals.emplace_back(Eigen::Vector3f());
	}

	//reconstruction.release();
}
