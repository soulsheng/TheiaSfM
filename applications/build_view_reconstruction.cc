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

#include "build_common.h"
#include "view_common.h"

// OpenGL camera parameters.
float zoom_default = -400.0;
float zoom = zoom_default;

// Rotation values for the navigation
Eigen::Vector2f navigation_rotation_default(60.0, 0.0);
Eigen::Vector2f navigation_rotation(navigation_rotation_default);

int n_fps = 240; // frame per second

Eigen::Vector2i window_position(200, 100);

int main(int argc, char* argv[]) {
  THEIA_GFLAGS_NAMESPACE::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  const ReconstructionBuilderOptions options =
	  SetReconstructionBuilderOptions();

  ReconstructionBuilder reconstruction_builder(options);
  // If matches are provided, load matches otherwise load images.
  if (FLAGS_matches_file.size() != 0) {
	  AddMatchesToReconstructionBuilder(&reconstruction_builder);
  }
  else if (FLAGS_images.size() != 0) {
	  AddImagesToReconstructionBuilder(&reconstruction_builder);
  }
  else {
	  LOG(FATAL)
		  << "You must specifiy either images to reconstruct or a match file.";
  }

  std::vector<Reconstruction*> reconstructions;
  CHECK(reconstruction_builder.BuildReconstruction(&reconstructions))
	  << "Could not create a reconstruction.";

  Reconstruction* reconstruction = NULL;
  if (reconstructions.size())
	  reconstruction = reconstructions[0];
  else
	  return -1;

  // Centers the reconstruction based on the absolute deviation of 3D points.
  reconstruction->Normalize();

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

  //reconstruction.release();

  // Set up opengl and glut.
  glutInit(&argc, argv);
  glutInitWindowPosition(window_position[0], window_position[1]);
  glutInitWindowSize(1200, 800);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutCreateWindow("Theia Reconstruction Viewer");

#ifdef _WIN32
  // Set up glew.
  CHECK_EQ(GLEW_OK, glewInit())
      << "Failed initializing GLEW.";
#endif

  // Set the camera
  gluLookAt(0.0f, 0.0f, -6.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

  // register callbacks
  glutDisplayFunc(RenderScene);
  glutReshapeFunc(ChangeSize);
  glutMouseFunc(MouseButton);
  glutMotionFunc(MouseMove);
  glutKeyboardFunc(Keyboard);
  glutIdleFunc(RenderScene);

  // enter GLUT event processing loop
  glutMainLoop();

  return 0;
}
