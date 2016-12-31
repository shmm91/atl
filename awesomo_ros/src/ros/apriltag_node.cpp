#include "awesomo_ros/apriltag_node.hpp"

namespace awesomo {

int AprilTagNode::configure(const std::string &node_name, int hz) {
  std::string config_file;

  // ros node
  ROSNode::configure(node_name, hz);

  // detector
  this->ros_nh->getParam("/apriltag_config", config_file);
  if (this->detector.configure(config_file) != 0) {
    return -1;
  };

  // subscribers and publishers
  // clang-format off
  ROSNode::registerPublisher<awesomo_msgs::AprilTagPose>(APRILTAG_POSE_TOPIC);
  ROSNode::registerSubscriber(CAMERA_IMAGE_TOPIC, &AprilTagNode::imageCallback, this);
  // clang-format on

  return 0;
}

void AprilTagNode::imageCallback(const sensor_msgs::ImageConstPtr &msg) {
  cv_bridge::CvImagePtr image_ptr;
  std::vector<TagPose> tags;
  awesomo_msgs::AprilTagPose tag_msg;

  // detect tags
  image_ptr = cv_bridge::toCvCopy(msg);
  tags = this->detector.extractTags(image_ptr->image);

  // publish tag pose
  if (tags.size()) {
    buildAprilTagPoseMsg(tags[0], tag_msg);
    this->ros_pubs[APRILTAG_POSE_TOPIC].publish(tag_msg);
  }
}

}  // end of awesomo namespace

int main(void) {
  awesomo::AprilTagNode node;
  node.configure(APRILTAG_NODE_NAME, APRILTAG_NODE_RATE);
  node.loop();
  return 0;
}