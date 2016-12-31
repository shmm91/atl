#ifndef __AWESOMO_ROS_CONTROL_NODE_HPP__
#define __AWESOMO_ROS_CONTROL_NODE_HPP__

#include <ros/ros.h>

#define MAVLINK_DIALECT common
#include <mavros/mavros.h>
#include <mavros_msgs/State.h>
#include <mavros_msgs/SetMode.h>
#include <mavros_msgs/CommandBool.h>
#include <mavros_msgs/RCIn.h>

#include <awesomo_msgs/KFStats.h>
#include <awesomo_msgs/KFPlotting.h>
#include <awesomo_msgs/AprilTagPose.h>
#include <awesomo_msgs/PositionControllerStats.h>

#include <awesomo_core/awesomo_core.hpp>

#include "awesomo_ros/ros_node.hpp"
#include "awesomo_ros/ros_msgs.hpp"


namespace awesomo {

// ROS NODE
#define CONTROL_NODE_NAME "awesomo_node"
#define CONTROL_NODE_RATE 1000

// ROS TOPICS
#define MAVROS_ARM_TOPIC "/mavros/cmd/arming"
#define MAVROS_MODE_TOPIC "/mavros/set_mode"

#define MAVROS_STATE_TOPIC "/mavros/state"
#define MAVROS_POSE_TOPIC "/mavros/local_position/pose"
#define MAVROS_RADIO_TOPIC "/mavros/rc/in"
#define AWESOMO_APRILTAG_TOPIC "/awesomo/vision/pose"

#define MAVROS_SETPOINT_ATTITUDE_TOPIC "/mavros/setpoint_attitude/attitude"
#define MAVROS_SETPOINT_THROTTLE_TOPIC \
  "/mavros/setpoint_attitude/att_throttle"
#define MAVROS_SETPOINT_POSITION_TOPIC "/mavros/setpoint_position/local"

#define AWESOMO_POS_CONTROLLER_STATS_TOPIC \
  "/awesomo/position_controller/stats"
#define AWESOMO_KF_STATS_TOPIC "/awesomo/kf_estimation/stats"
#define AWESOMO_KF_PLOTTING_TOPIC "/awesomo/kf_estimation/states"

class ControlNode : public ROSNode {
public:
  bool configured;

  Quadrotor quadrotor;

  Pose world_pose;
  mavros_msgs::State mavros_state;
  int rc_in[16];
  TagPose tag_pose;

  ros::ServiceClient mode_client;
  ros::ServiceClient arming_client;

  ControlNode(void);
  int configure(std::string node_name, int hz);
  void waitForConnection(void);
  int disarm(void);
  int setOffboardModeOn(void);
  void stateCallback(const mavros_msgs::State::ConstPtr &msg);
  void poseCallback(const geometry_msgs::PoseStamped &msg);
  void radioCallback(const mavros_msgs::RCIn &msg);
  void aprilTagCallback(const awesomo_msgs::AprilTagPose &msg);
  int awesomoLoopCallback(void);
  void publishStats(void);
};

}  // end of awesomo namespace
#endif