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

#include "theia/image/descriptor/create_descriptor_extractor.h"

#include <glog/logging.h>
#include <memory>

#include "theia/image/descriptor/akaze_descriptor.h"
#include "theia/image/descriptor/descriptor_extractor.h"
#include "theia/image/descriptor/sift_descriptor.h"
#include "theia/image/keypoint_detector/sift_parameters.h"

namespace theia {
namespace {
inline SiftParameters FeatureDensityToSiftParameters(
    const FeatureDensity& density) {
  SiftParameters sift_params;
  if (density == FeatureDensity::DENSE) {
    sift_params.edge_threshold *= 2.0;
    sift_params.peak_threshold /= 3.0;
  } else if (density == FeatureDensity::SPARSE) {
    sift_params.edge_threshold /= 2.0;
    sift_params.peak_threshold *= 3.0;
  } else if (density == FeatureDensity::DENSE_HALF) {
    sift_params.peak_threshold /= 1.5;
  } else if (density == FeatureDensity::SPARSE_HALF) {
    sift_params.peak_threshold *= 1.5;  
  } else if (density != FeatureDensity::NORMAL) {
    // If the setting is to normal, then just use the default
    // parameters. Otherwise, this statement will be reached, indicating that an
    // invalid option was used.
    LOG(FATAL) << "Invalid feature extraction density. Please use DENSE, "
                  "NORMAL, or SPARSE.";
  }
  return sift_params;
}

inline AkazeParameters FeatureDensityToAkazeParameters(
    const FeatureDensity& density) {
  AkazeParameters akaze_params;
  if (density == FeatureDensity::DENSE) {
    akaze_params.hessian_threshold /= 10.0;
  } else if (density == FeatureDensity::SPARSE) {
    akaze_params.hessian_threshold *= 10.0;
  } else if (density != FeatureDensity::NORMAL) {
    // If the setting is to normal, then just use the default
    // parameters. Otherwise, this statement will be reached, indicating that an
    // invalid option was used.
    LOG(FATAL) << "Invalid feature extraction density. Please use DENSE, "
                  "NORMAL, or SPARSE.";
  }
  return akaze_params;
}

}  // namespace

std::unique_ptr<DescriptorExtractor> CreateDescriptorExtractor(
    const DescriptorExtractorType& descriptor_type,
    const FeatureDensity& feature_density) {
  std::unique_ptr<DescriptorExtractor> descriptor_extractor;
  switch (descriptor_type) {
    case DescriptorExtractorType::SIFT:
      descriptor_extractor.reset(new SiftDescriptorExtractor(
          FeatureDensityToSiftParameters(feature_density)));
      break;
    case DescriptorExtractorType::AKAZE:
      descriptor_extractor.reset(new AkazeDescriptorExtractor(
          FeatureDensityToAkazeParameters(feature_density)));
      break;
    default:
      LOG(ERROR) << "Invalid Descriptor Extractor specified.";
  }
  CHECK(descriptor_extractor->Initialize())
      << "Could not initialize the Descriptor Extractor";
  return descriptor_extractor;
}

}  // namespace theia
