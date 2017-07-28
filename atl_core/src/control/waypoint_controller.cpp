#include "atl/control/waypoint_controller.hpp"

namespace atl {

int WaypointController::configure(const std::string &config_file) {
  ConfigParser parser;

  // load config
  parser.addParam("at_controller.k_p", &this->at_controller.k_p);
  parser.addParam("at_controller.k_i", &this->at_controller.k_i);
  parser.addParam("at_controller.k_d", &this->at_controller.k_d);
  parser.addParam("ct_controller.k_p", &this->ct_controller.k_p);
  parser.addParam("ct_controller.k_i", &this->ct_controller.k_i);
  parser.addParam("ct_controller.k_d", &this->ct_controller.k_d);
  parser.addParam("z_controller.k_p", &this->z_controller.k_p);
  parser.addParam("z_controller.k_i", &this->z_controller.k_i);
  parser.addParam("z_controller.k_d", &this->z_controller.k_d);
  parser.addParam("yaw_controller.k_p", &this->yaw_controller.k_p);
  parser.addParam("yaw_controller.k_i", &this->yaw_controller.k_i);
  parser.addParam("yaw_controller.k_d", &this->yaw_controller.k_d);

  parser.addParam("roll_limit.min", &this->roll_limit[0]);
  parser.addParam("roll_limit.max", &this->roll_limit[1]);
  parser.addParam("pitch_limit.min", &this->pitch_limit[0]);
  parser.addParam("pitch_limit.max", &this->pitch_limit[1]);
  parser.addParam("hover_throttle", &this->hover_throttle);
  if (parser.load(config_file) != 0) {
    return -1;
  }

  // convert roll and pitch limits from degrees to radians
  this->roll_limit[0] = deg2rad(this->roll_limit[0]);
  this->roll_limit[1] = deg2rad(this->roll_limit[1]);
  this->pitch_limit[0] = deg2rad(this->pitch_limit[0]);
  this->pitch_limit[1] = deg2rad(this->pitch_limit[1]);

  this->configured = true;
  return 0;
}

int WaypointController::update(const Pose &pose,
                               const Vec3 &vel,
                               const double dt,
                               Vec4 &u) {
  // check rate
  this->dt += dt;
  if (this->dt < 0.01) {
    u = this->outputs;
    return 0;
  }

  // current waypoint
  Vec3 waypoint;
  // int retval = this->waypointUpdate(pose.position, waypoint);
  // if (retval != 0) {
  //   u = this->outputs;
  //   return retval;
  // }
  //
  // // roll
  // double error_ct =
  //   this->crossTrackError(this->wp_start, this->wp_end, pose.position);
  // double r = this->ct_controller.update(error_ct, this->dt);
  //
  // // pitch
  // Vec3 tu = this->waypointTangentUnitVector(this->wp_end, this->wp_start);
  // double error_at = vel_desired - vel.dot(tu);
  // double p = this->at_controller.update(error_at, this->dt);
  //
  // // yaw
  // Vec3 euler;
  // quat2euler(pose.orientation, 321, euler);
  // double yaw_setpoint = this->waypointHeading(this->wp_start,
  // this->wp_end);
  // double y = this->yaw_controller.update(yaw_setpoint, euler(2), this->dt);
  //
  // // throttle
  // double error_z = waypoint(2) - pose.position(2);
  // double t = this->hover_throttle;
  // t += this->z_controller.update(error_z, this->dt);
  // t /= fabs(cos(r) * cos(p));  // adjust throttle for roll and pitch

  double r, p, y, t;

  // limit roll, pitch, throttle
  r = (r < this->roll_limit[0]) ? this->roll_limit[0] : r;
  r = (r > this->roll_limit[1]) ? this->roll_limit[1] : r;
  p = (p < this->pitch_limit[0]) ? this->pitch_limit[0] : p;
  p = (p > this->pitch_limit[1]) ? this->pitch_limit[1] : p;
  t = (t < 0.0) ? 0.0 : t;
  t = (t > 1.0) ? 1.0 : t;

  // keep track of setpoints and outputs
  this->outputs << r, p, y, t;
  this->dt = 0.0;

  return 0;
}

}  // namespace atl