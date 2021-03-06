// Copyright (C) 2015 The Regents of the University of California (Regents).
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

#include "theia/sfm/feature_extractor_and_matcher.h"

#include <Eigen/Core>
#include <glog/logging.h>
#include <algorithm>
#include <memory>
#include <string>
#include <thread>  // NOLINT
#include <vector>

#include "theia/image/image.h"
#include "theia/image/descriptor/create_descriptor_extractor.h"
#include "theia/image/descriptor/descriptor_extractor.h"
#include "theia/image/keypoint_detector/keypoint.h"
#include "theia/matching/create_feature_matcher.h"
#include "theia/matching/feature_correspondence.h"
#include "theia/matching/feature_matcher_options.h"
#include "theia/matching/image_pair_match.h"
#include "theia/sfm/camera_intrinsics_prior.h"
#include "theia/sfm/estimate_twoview_info.h"
#include "theia/sfm/exif_reader.h"
#include "theia/sfm/two_view_match_geometric_verification.h"
#include "theia/util/filesystem.h"
#include "theia/util/string.h"
#include "theia/util/threadpool.h"
#include "theia/util/timer.h"

//#define USE_GPU			1
#define		TEST_IMAGE		"640-2.jpg"	// in root path 

namespace theia {
namespace {

void ExtractFeatures(
    const FeatureExtractorAndMatcher::Options& options,
    const std::string& image_filepath,
    const std::string& imagemask_filepath,
    std::vector<Keypoint>* keypoints,
    std::vector<Eigen::VectorXf>* descriptors) {
  static const float kMaskThreshold = 0.5;
  std::unique_ptr<FloatImage> image(new FloatImage(image_filepath));
  // We create these variable here instead of upon the construction of the
  // object so that they can be thread-safe. We *should* be able to use the
  // static thread_local keywords, but apparently Mac OS-X's version of clang
  // does not actually support it!
  //
  // TODO(cmsweeney): Change this so that each thread in the threadpool receives
  // exactly one object.
  std::unique_ptr<DescriptorExtractor> descriptor_extractor =
      CreateDescriptorExtractor(options.descriptor_extractor_type,
                                options.feature_density);

  // Exit if the descriptor extraction fails.
  if (!descriptor_extractor->DetectAndExtractDescriptors(*image,
                                                         keypoints,
                                                         descriptors)) {
    LOG(ERROR) << "Could not extract descriptors in image " << image_filepath;
    return;
  }

  if (imagemask_filepath.size() > 0) {
    std::unique_ptr<FloatImage> image_mask(new FloatImage(imagemask_filepath));
    // Check the size of the image and its associated mask.
    CHECK(image_mask->Width() == image->Width() &&
          image_mask->Height() == image->Height())
      << "The image and the mask don't have the same size. \n"
      << "- Image: " << image_filepath
      << "\t(" << image->Width() << " x " << image->Height() << ")\n"
      << "- Mask: " << imagemask_filepath
      << "\t(" << image_mask->Width() << " x " << image_mask->Height() << ")";

    // Convert the mask to grayscale.
    image_mask->ConvertToGrayscaleImage();
    // Remove keypoints according to the associated mask (remove kp. in black
    // part).
    for (int i=keypoints->size()-1; i>-1; i--) {
      if (image_mask->BilinearInterpolate(keypoints->at(i).x(),
                                          keypoints->at(i).y(),
                                          0) < kMaskThreshold) {
        keypoints->erase(keypoints->begin() + i);
        descriptors->erase(descriptors->begin() + i);
      }
    }
  }

  if (keypoints->size() > options.max_num_features) {
    keypoints->resize(options.max_num_features);
    descriptors->resize(options.max_num_features);
  }

  if (imagemask_filepath.size() > 0) {
    VLOG(1) << "Successfully extracted " << descriptors->size()
            << " features from image " << image_filepath
            << " with an image mask.";
  } else {
    VLOG(1) << "Successfully extracted " << descriptors->size()
            << " features from image " << image_filepath;
  }
}

}  // namespace

FeatureExtractorAndMatcher::FeatureExtractorAndMatcher(
	const FeatureExtractorAndMatcher::Options& options, std::string exePath, bool use_gpu, bool bSilence)
    : options_(options)
	, exePath_(exePath) {
  // Create the feature matcher.
  FeatureMatcherOptions matcher_options = options_.feature_matcher_options;
  matcher_options.num_threads = options_.num_threads;
  matcher_options.min_num_feature_matches = options_.min_num_inlier_matches;
  matcher_options.perform_geometric_verification = true;
  matcher_options.geometric_verification_options.min_num_inlier_matches =
      options_.min_num_inlier_matches;

  matcher_ = CreateFeatureMatcher(options_.matching_strategy, matcher_options, bSilence);
  exif_reader_.LoadSensorWidthDatabase(exePath_);

  this->bSilence = bSilence;
  this->use_gpu = use_gpu;
  
  if (this->use_gpu)
  {
	  // 输出信息简化 
	  char * argv[] = { "-v", "0", "-fo", "-1", "-t", "0.004444", "-e", "10" };
	  int argc = sizeof(argv) / sizeof(char*);
	  char **ppChar = new char*[argc];
	  for (int i = 0; i < argc; i++)
	  {
		  ppChar[i] = new char[16];
		  strcpy(ppChar[i], argv[i]);
	  }

	  // custom sift parameter
	  float dog_threshold = 0.02 / sift._dog_level_num; // default gpu sift get too few features
	  dog_threshold /= 1.5f;	// to get more features

	  float edge_threshold = 10.0f;

	  switch (options_.feature_density)
	  {
	  case FeatureDensity::SPARSE:
		  edge_threshold /= 2.0;
		  dog_threshold *= 3.0;
		  break;

	  case FeatureDensity::DENSE:
		  edge_threshold *= 2.0;
		  dog_threshold /= 3.0;
		  break;

	  case FeatureDensity::SPARSE_HALF:
		  dog_threshold *= 1.5;
		  break;

	  case FeatureDensity::DENSE_HALF:
		  dog_threshold /= 1.5;
		  break;

	  default:
		  break;
	  }

	  sprintf(ppChar[5], "%f", dog_threshold); 
	  sprintf(ppChar[7], "%f", edge_threshold);
	  sift.ParseParam(argc, ppChar);

	  if (sift.CreateContextGL() != SiftGPU::SIFTGPU_FULL_SUPPORTED ||
		  false == sift.RunSIFT(TEST_IMAGE) ||
		  0 == sift.GetFeatureNum())
	  {
		  this->use_gpu = false;
		  LOG(INFO) << "GPU不支持，改用CPU运行SIFT...";
	  }
	  else
		  LOG(INFO) << "GPU支持，采用GPU运行SIFT...";
  }
  else
	  LOG(INFO) << "采用CPU运行SIFT...";

}

bool FeatureExtractorAndMatcher::AddImage(const std::string& image_filepath) {
  image_filepaths_.emplace_back(image_filepath);
  return true;
}

bool FeatureExtractorAndMatcher::AddImage(
    const std::string& image_filepath,
    const CameraIntrinsicsPrior& intrinsics) {
  if (!AddImage(image_filepath)) {
    return false;
  }
  intrinsics_[image_filepath] = intrinsics;
  return true;
}

bool FeatureExtractorAndMatcher::AddMaskForFeaturesExtraction(
    const std::string& image_filepath,
    const std::string& mask_filepath) {
  image_masks_[image_filepath] = mask_filepath;
  VLOG(1) << "Image: " << image_filepath << " || "
          << "Associated mask: " << mask_filepath;
  return true;
}

// Performs feature matching between all images provided by the image
// filepaths. Features are extracted and matched between the images according to
// the options passed in. Only matches that have passed geometric verification
// are kept. EXIF data is parsed to determine the camera intrinsics if
// available.
int FeatureExtractorAndMatcher::ExtractAndMatchFeatures(
    std::vector<CameraIntrinsicsPrior>* intrinsics,
    std::vector<ImagePairMatch>* matches) {
  CHECK_NOTNULL(intrinsics)->resize(image_filepaths_.size());
  CHECK_NOTNULL(matches);
  CHECK_NOTNULL(matcher_.get());
  if (!use_gpu)// cpu 多线程
  { 
  // For each image, process the features and add it to the matcher.
  const int num_threads =
      std::min(options_.num_threads, static_cast<int>(image_filepaths_.size()));
  std::unique_ptr<ThreadPool> thread_pool(new ThreadPool(num_threads));
  for (int i = 0; i < image_filepaths_.size(); i++) {
    if (!FileExists(image_filepaths_[i])) {
      LOG(ERROR) << "Could not extract features for " << image_filepaths_[i]
                 << " because the file cannot be found.";
      continue;
    }
    thread_pool->Add(&FeatureExtractorAndMatcher::ProcessImage, this, i);
  }
  // This forces all tasks to complete before proceeding.
  thread_pool.reset(nullptr);
  }
  else// gpu 单线程
  {
	  for (int i = 0; i < image_filepaths_.size(); i++)
	  {
		  ProcessImage(i);
	  }
  }
  int nRetCode = 0;

  if ( matcher_->NullFeatures() )
  {
	  nRetCode = -23;

	  LOG(INFO) << "异常返回！异常代码：" << nRetCode << std::endl
		  << "异常描述：特征点检测失败，可能原因：GPU不支持，建议措施：不勾选GPU";

	  return nRetCode;
  }

  // After all threads complete feature extraction, perform matching.

  // Perform the matching.
  LOG(INFO) << "特征匹配（Matching images）...";
  matcher_->MatchImages(matches);

  // Add the intrinsics to the output.
  for (int i = 0; i < image_filepaths_.size(); i++) {
    (*intrinsics)[i] = FindOrDie(intrinsics_, image_filepaths_[i]);
  }

  return nRetCode;
}

void FeatureExtractorAndMatcher::ProcessImage(
    const int i) {
  const std::string& image_filepath = image_filepaths_[i];

  // Get the camera intrinsics prior if it was provided.
  CameraIntrinsicsPrior intrinsics =
      FindWithDefault(intrinsics_, image_filepath, CameraIntrinsicsPrior());

  // Get the associated mask if it was provided.
  const std::string mask_filepath =
      FindWithDefault(image_masks_, image_filepath, "");

  // Extract an EXIF focal length if it was not provided.
  if (!intrinsics.focal_length.is_set) {
    CHECK(exif_reader_.ExtractEXIFMetadata(image_filepath, &intrinsics));

    // If the focal length still could not be extracted, set it to a reasonable
    // value based on a median viewing angle.
    if (!options_.only_calibrated_views && !intrinsics.focal_length.is_set) {
      VLOG(2) << "Exif was not detected. Setting it to a reasonable value.";
      intrinsics.focal_length.is_set = true;
      intrinsics.focal_length.value[0] =
          1.2 * static_cast<double>(
                    std::max(intrinsics.image_width, intrinsics.image_height));
    }
	if (!use_gpu)
	{
		std::lock_guard<std::mutex> lock(intrinsics_mutex_);
		intrinsics_[image_filepath] = intrinsics;
	}
    // Insert or update the value of the intrinsics.
	else
		intrinsics_[image_filepath] = intrinsics;
  }

  // Early exit if no EXIF calibration exists and we are only processing
  // calibration views.
  if (options_.only_calibrated_views && !intrinsics.focal_length.is_set) {
    LOG(INFO) << "Image " << image_filepath
              << " did not contain an EXIF focal length. Skipping this image.";
    return;
  } 
#if 0
  else {
	  LOG(INFO) << "Image ";
	  LOG(INFO) << image_filepath;
	  LOG(INFO) << " is initialized with the focal length: ";
	  //LOG(INFO) << intrinsics.focal_length.value[0];
  }
#endif

  // Get the image filename without the directory.
  std::string image_filename;
  CHECK(GetFilenameFromFilepath(image_filepath, true, &image_filename));

  // Get the feature filepath based on the image filename.
  std::string output_dir =
      options_.feature_matcher_options.keypoints_and_descriptors_output_dir;
  AppendTrailingSlashIfNeeded(&output_dir);
  const std::string feature_filepath =
      output_dir + image_filename + ".features";

  // If the feature file already exists, skip the feature extraction.
  if (options_.feature_matcher_options.match_out_of_core &&
      FileExists(feature_filepath)) {
	  if (!use_gpu)
	  {
		  std::lock_guard<std::mutex> lock(matcher_mutex_);
		  matcher_->AddImage(image_filename, intrinsics);
	  }
	  else
		  matcher_->AddImage(image_filename, intrinsics);
    return;
  }

  Timer timer;
  timer.Reset();

  // Extract Features.
  std::vector<Keypoint> keypoints;
  std::vector<Eigen::VectorXf> descriptors;

  if(!use_gpu)

  ExtractFeatures(options_,
                  image_filepath,
                  mask_filepath,
                  &keypoints,
                  &descriptors);

  else {

	  std::vector<float> descriptors_h(1);
	  std::vector<SiftGPU::SiftKeypoint> keys_h(1);

	  sift.RunSIFT(image_filepath.c_str());

	  //get feature count
	  int num1 = sift.GetFeatureNum();
	  //allocate memory
	  keys_h.resize(num1);    descriptors_h.resize(128 * num1);

	  sift.GetFeatureVector(&keys_h[0], &descriptors_h[0]);

	  for (int i = 0; i < keys_h.size(); i++)
	  {
		  Keypoint kp;
		  kp.set_x(keys_h[i].x);
		  kp.set_y(keys_h[i].y);
		  kp.set_scale(keys_h[i].s);
		  kp.set_orientation(keys_h[i].o);
		  keypoints.push_back(kp);

		  descriptors.emplace_back(128);
		  memcpy(descriptors.back().data(), descriptors_h.data() + i * 128, sizeof(float) * 128);
	  }

  }

  if (!bSilence)
  {
	  std::cout << "ExtractFeatures time " << timer.ElapsedTimeInSeconds() << " Seconds" << std::endl;
	  std::cout << i << " - " << image_filepath << " 特征点数目是: " << keypoints.size() << std::endl;
  }

  LOG(INFO) << i << " - " << image_filepath << " 特征点数目是: " << keypoints.size();

  // Add the relevant image and feature data to the feature matcher. This allows
  // the feature matcher to control fine-grained things like multi-threading and
  // caching. For instance, the matcher may choose to write the descriptors to
  // disk and read them back as needed.
  if (!use_gpu)
  {
	  std::lock_guard<std::mutex> lock(matcher_mutex_);
	  matcher_->AddImage(image_filename, keypoints, descriptors, intrinsics);
  }
  else
      matcher_->AddImage(image_filename, keypoints, descriptors, intrinsics);
}

}  // namespace theia
