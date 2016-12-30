#ifndef __AWESOMO_VISION_APRILTAG_SWATHMORE_HPP__
#define __AWESOMO_VISION_APRILTAG_SWATHMORE_HPP__

#include <cmath>
#include <fstream>
#include <iostream>
#include <math.h>
#include <libgen.h>
#include <sys/time.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <apriltags/TagDetector.h>
#include <apriltags/CameraUtil.h>

#include "awesomo_core/utils/utils.hpp"
#include "awesomo_core/vision/camera/camera.hpp"
#include "awesomo_core/vision/apriltag/data.hpp"
// #include "awesomo_core/vision/estimator.hpp"


namespace awesomo {

class SwathmoreDetector {
public:
  bool configured;

  TagDetector *detector;

  std::map<int, float> tag_configs;
  std::string camera_mode;
  std::vector<std::string> camera_modes;
  std::map<std::string, CameraConfig *> camera_configs;
  bool imshow;

  SwathmoreDetector(void);
  ~SwathmoreDetector(void);
  int configure(std::string config_file);
  int obtainPose(TagDetection tag, TagPose &tag_pose);
  std::vector<TagPose> extractTags(cv::Mat &image);
  void printTag(TagPose tag);
};

}  // end of awesomo namespace
#endif
