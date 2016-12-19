#include "awesomo_core/quadrotor.hpp"


LandingConfig::LandingConfig(void) {
  this->period = 0;
  this->descend_multiplier = 0;
  this->recover_multiplier = 0;
  this->cutoff_position << 0, 0, 0;
  this->belief_threshold = 0;
}

LandingConfig::LandingConfig(float period,
                             float desend_multiplier,
                             float recover_multiplier,
                             float belief_threshold,
                             Eigen::Vector3d cutoff_position) {
  this->period = period;
  this->descend_multiplier = descend_multiplier;
  this->recover_multiplier = recover_multiplier;
  this->cutoff_position = cutoff_position;
  this->belief_threshold = belief_threshold;
}

Quadrotor::Quadrotor(std::map<std::string, std::string> configs) {
  std::string config_path;

  // precheck
  if (configs.count("quadrotor") == 0) {
    std::cout << "ERROR! quadrotor config not set!" << std::endl;
  }

  // configs
  this->landing_config = new LandingConfig();

  // intialize state
  this->mission_state = DISCOVER_MODE;
  this->world_pose.position << 0.0, 0.0, 0.0;
  this->yaw = 0;

  // landing state
  this->landing_belief = 0;

  // estimators
  this->estimator_initialized = false;

  // initialize position controller
  if (configs.count("position_controller")) {
    config_path = configs["position_controller"];
    this->position_controller = new PositionController(config_path);
  } else {
    this->position_controller = NULL;
  }

  // load quadrotor configuration
  config_path = configs["quadrotor"];
  this->loadConfig(config_path);
}

int Quadrotor::loadConfig(std::string config_file_path) {
  YAML::Node config;
  YAML::Node tracking;
  YAML::Node landing;

  try {
    // load config
    config = YAML::LoadFile(config_file_path);

    // height offset
    this->height_offset_initialized = false;
    this->height_offset = 0.0f;

    // load hover height config
    this->hover_height_original = config["hover_height"].as<float>();
    this->hover_height = config["hover_height"].as<float>();

    // load landing config
    landing = config["landing"]["height_update"];
    this->landing_config->period = landing["period"].as<float>();

    landing = config["landing"]["height_update"]["multiplier"];
    this->landing_config->descend_multiplier = landing["descend"].as<float>();
    this->landing_config->recover_multiplier = landing["recover"].as<float>();

    landing = config["landing"]["disarm_conditions"];
    this->landing_config->cutoff_position << landing["x_cutoff"].as<float>(),
      landing["y_cutoff"].as<float>(), landing["z_cutoff"].as<float>();

    this->landing_config->belief_threshold =
      landing["belief_threshold"].as<float>();

  } catch (YAML::BadFile &ex) {
    printf("ERROR! invalid quadrotor configuration file!\n");
    throw;
  }

  return 0;
}

Attitude Quadrotor::positionControllerCalculate(Eigen::Vector3d setpoint,
                                                Pose robot_pose,
                                                float yaw,
                                                float dt) {
  Attitude a;
#ifdef YAW_CONTROL_ON
  this->position_controller->calculate(setpoint, robot_pose, yaw, dt);
#else
  this->position_controller->calculate(setpoint, robot_pose, 0.0, dt);
#endif

  a.x = this->position_controller->command_quat.x();
  a.y = this->position_controller->command_quat.y();
  a.z = this->position_controller->command_quat.z();
  a.w = this->position_controller->command_quat.w();

  a.roll = this->position_controller->roll;
  a.pitch = this->position_controller->pitch;
  a.yaw = yaw;

  return a;
}

void Quadrotor::resetPositionController(void) {
  this->position_controller->reset();
}

int Quadrotor::calculateLandingTargetYaw(double *yaw) {
  int retval;
  double x_median;
  double y_median;
  double relative_yaw;
  Eigen::Vector2d p;
  std::vector<double> x_values;
  std::vector<double> y_values;

  // obtain median landing target x, y
  for (int i = 0; i < this->lt_history.size(); i++) {
    p = this->lt_history[i];
    x_values.push_back(p(0));
    y_values.push_back(-1 * p(1));  // -ve because NED y is +ve right
  }
  std::sort(x_values.begin(), x_values.end());
  std::sort(y_values.begin(), y_values.end());

  if (x_values.size() > 1) {
    x_median = x_values[x_values.size() / 2];
    y_median = y_values[y_values.size() / 2];

  } else {
    x_median = x_values[0];
    y_median = y_values[0];
  }

  // calculate relative yaw
  relative_yaw = atan2(y_median, x_median);

  // convert relative yaw to global yaw
  *yaw = this->yaw + relative_yaw;
  if (*yaw < 0) {
    *yaw += 2 * M_PI;
  } else if (*yaw > 2 * M_PI) {
    *yaw -= 2 * M_PI;
  }

  return 0;
}

void Quadrotor::runDiscoverMode(LandingTargetPosition landing) {
  Eigen::VectorXd mu(9);
  Eigen::Vector2d lt_pos;
  double lt_yaw;
  bool transition_state;

  // setup
  transition_state = false;

  // obtain landing target yaw
  if (landing.detected == true) {
    if (lt_history.size() < 10) {
      lt_pos << landing.position(0), landing.position(1);
      lt_history.push_back(lt_pos);

    } else if (lt_history.size() == 10) {
      // calculate landing target yaw
      this->calculateLandingTargetYaw(&lt_yaw);
      transition_state = true;
      printf("LANDING TARGET YAW IS: %.2f deg\n", rad2deg(lt_yaw));

      // set quadrotor yaw
      this->yaw = lt_yaw;

      // clear landing target history
      lt_history.clear();
    }
  }

  // initialize landing target estimator
  if (transition_state) {
    mu << landing.position(0),  // pos_x relative to quad
      landing.position(1),      // pos_y relative to quad
      landing.position(2),      // pos_z relative to quad
      0.0, 0.0, 0.0,            // vel_x, vel_y, vel_z
      0.0, 0.0, 0.0;            // acc_x, acc_y, acc_z
    apriltag_kf_setup(&this->tag_estimator, mu);
    this->estimator_initialized = true;

    // transition to tracker mode
    printf("Transitioning to TRACKER MODE!\n");
    this->mission_state = TRACKING_MODE;
    tic(&this->tracking_start);
    tic(&this->target_last_updated);

    // reset position controller softly so not initially violent
    this->position_controller->x.prev_error = landing.position(1);
    this->position_controller->y.prev_error = landing.position(0);
  }
}

void Quadrotor::runTrackingModeBPF(LandingTargetPosition landing, float dt) {
  Eigen::Vector3d tag_mea;
  Eigen::Vector3d setpoint;
  Pose robot_pose;
  float tag_x;
  float tag_y;
  float elasped;

  // estimate tag position
  tag_mea = landing.position;
  apriltag_kf_estimate(&this->tag_estimator, tag_mea, dt, landing.detected);

  // keep track of target position
  elasped = toc(&this->target_last_updated);
  if (landing.detected == true) {
    tic(&this->target_last_updated);
  } else if (elasped > 1.0) {
    printf("Target losted transitioning back to DISCOVER MODE!\n");
    this->mission_state = DISCOVER_MODE;
  }

  // setpoint
  tag_x = this->tag_estimator.mu(0);
  tag_y = this->tag_estimator.mu(1);
  setpoint << tag_x, tag_y, this->hover_height;

  // robot pose
  robot_pose.position(0) = 0.0;
  robot_pose.position(1) = 0.0;
  robot_pose.position(2) = this->world_pose.position(2);
  robot_pose.q = this->world_pose.q;

  // update position controller
  this->positionControllerCalculate(setpoint, robot_pose, this->yaw, dt);

  // transition to landing
  elasped = toc(&this->tracking_start);
  if (elasped > 5.0) {  // track for 10 seconds then land
    printf("Transitioning to LANDING MODE!\n");
    this->mission_state = LANDING_MODE;
    tic(&this->height_last_updated);
  }
}

bool Quadrotor::withinLandingZone(Eigen::Vector3d &m, Eigen::Vector3d &e) {
  Eigen::Vector3d threshold;
  bool measured_x_ok;
  bool measured_y_ok;
  bool measured_z_ok;
  bool estimated_x_ok;
  bool estimated_y_ok;
  bool estimated_z_ok;

  threshold = this->landing_config->cutoff_position;
  measured_x_ok = (m(0) < threshold(0)) ? true : false;
  measured_y_ok = (m(1) < threshold(1)) ? true : false;
  measured_z_ok = (m(2) < threshold(2)) ? true : false;
  estimated_x_ok = (e(0) < threshold(0)) ? true : false;
  estimated_y_ok = (e(1) < threshold(1)) ? true : false;
  estimated_z_ok = (e(2) < threshold(2)) ? true : false;

  // check measured
  if (measured_x_ok && measured_y_ok && measured_z_ok) {
    return true;

    // check estimated
  } else if (estimated_x_ok && estimated_y_ok && estimated_z_ok) {
    return true;

    // not near landing zone
  } else {
    return false;
  }
}

int Quadrotor::checkLandingTargetEstimation(Eigen::Vector3d &est) {
  if (est(0) > 5.0 || est(1) > 5.0) {
    return -1;
  }

  return 0;
}

void Quadrotor::runLandingMode(LandingTargetPosition landing, float dt) {
  Eigen::Vector3d tag_mea;
  Eigen::Vector3d tag_est;
  Eigen::VectorXd mu(9);
  Eigen::Vector3d threshold;
  Pose robot_pose;
  float elasped;

  // estimate tag position
  tag_mea = landing.position;
  apriltag_kf_estimate(&this->tag_estimator, tag_mea, dt, landing.detected);
  mu = this->tag_estimator.mu;
  tag_est << mu(0), mu(1), this->hover_height;

  // check landing target estimation
  if (this->checkLandingTargetEstimation(tag_est) == -1) {
    printf("Target losted transitioning back to DISCOVER MODE!\n");
    this->mission_state = DISCOVER_MODE;

    // reset hover height and landing belief
    this->hover_height = this->hover_height_original;
    this->landing_belief = 0;
  }

  // keep track of target position
  elasped = toc(&this->target_last_updated);
  if (landing.detected == true) {
    tic(&this->target_last_updated);

  } else if (elasped > 1.0) {
    printf("Target losted transitioning back to DISCOVER MODE!\n");
    this->mission_state = DISCOVER_MODE;

    // reset hover height and landing belief
    this->hover_height = this->hover_height_original;
    this->landing_belief = 0;
  }

  // landing - lower height or increase height
  elasped = toc(&this->height_last_updated);
  threshold = this->landing_config->cutoff_position;
  if (elasped > this->landing_config->period && landing.detected == true) {
    if (tag_mea(0) < threshold(0) && tag_mea(1) < threshold(1)) {
      this->hover_height *= this->landing_config->descend_multiplier;
      printf("Lowering hover height to %f\n", this->hover_height);

    } else {
      this->hover_height *= this->landing_config->recover_multiplier;
      printf("Increasing hover height to %f\n", this->hover_height);
    }
    tic(&this->height_last_updated);
  }

  // kill engines (landed?)
  if (this->withinLandingZone(tag_mea, tag_est)) {
    if (this->landing_belief >= this->landing_config->belief_threshold) {
      printf("MISSION ACCOMPLISHED!\n");
      this->mission_state = MISSION_ACCOMPLISHED;
    } else {
      this->landing_belief++;
    }
  }

  // robot pose
  robot_pose.position(0) = 0.0;
  robot_pose.position(1) = 0.0;
  robot_pose.position(2) = this->world_pose.position(2);
  robot_pose.q = this->world_pose.q;

  // update position controller
  this->positionControllerCalculate(tag_est, robot_pose, this->yaw, dt);
}

int Quadrotor::runMission(Pose world_pose,
                          LandingTargetPosition landing,
                          float dt) {
  // update pose
  this->world_pose = world_pose;

  // mission
  switch (this->mission_state) {
    case IDLE_MODE:
      this->resetPositionController();
      break;

    case DISCOVER_MODE:
      this->runDiscoverMode(landing);
      break;

    case TRACKING_MODE:
      this->runTrackingModeBPF(landing, dt);
      break;

    case LANDING_MODE:
      this->runLandingMode(landing, dt);
      break;

    case MISSION_ACCOMPLISHED:
      return 0;
      break;
  }

  return this->mission_state;
}