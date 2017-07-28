#ifndef ATL_MISSION_WAYPOINT_LINE_HPP
#define ATL_MISSION_WAYPOINT_LINE_HPP

#include "atl/utils/utils.hpp"
#include "atl/mission/waypoint.hpp"

namespace atl {

class WaypointLine {
public:
  Waypoint start;
  Waypoint end;

  WaypointLine() {}
};

}  // namespace atl

#endif
