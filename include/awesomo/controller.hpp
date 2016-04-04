#ifndef __CONTROLLER_HPP__
#define __CONTROLLER_HPP__

#include <iostream>
#include <deque>

#include <Eigen/Dense>


class CarrotController
{
    public:
        std::deque<Eigen::Vector3d> waypoints;
        int initialized;
        double look_ahead_dist;
        double wp_threshold;
        Eigen::Vector3d wp_start;
        Eigen::Vector3d wp_end;

        CarrotController();
        CarrotController(double look_ahead_dist, double wp_threshold);
        Eigen::Vector3d closestPoint(
            Eigen::Vector3d position,
            Eigen::Vector3d wp_start,
            Eigen::Vector3d wp_end
        );
        Eigen::Vector3d calculateCarrotPoint(
            Eigen::Vector3d position,
            double r,
            Eigen::Vector3d wp_start,
            Eigen::Vector3d wp_end
        );
        int waypointReached(
            Eigen::Vector3d position,
            Eigen::Vector3d waypoint,
            double threshold
        );
        int update(Eigen::Vector3d position, Eigen::Vector3d &carrot);
};

#endif
