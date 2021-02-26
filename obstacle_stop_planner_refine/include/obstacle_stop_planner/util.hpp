// Copyright 2019 Autoware Foundation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifndef OBSTACLE_STOP_PLANNER__UTIL_HPP_
#define OBSTACLE_STOP_PLANNER__UTIL_HPP_

#include "opencv2/core/core.hpp"
#include "geometry_msgs/msg/point.hpp"
#include "boost/geometry.hpp"

namespace bg = boost::geometry;
using Point = bg::model::d2::point_xy<double>;
using Polygon = bg::model::polygon<Point, false>;
using Line = bg::model::linestring<Point>;

namespace
{
cv::Point2d calcCentroid(const std::vector<cv::Point2d> pointcloud)
{
  cv::Point2d centroid;
  centroid.x = 0;
  centroid.y = 0;
  for (const auto &point : pointcloud)
  {
    centroid.x += point.x;
    centroid.y += point.y;
  }
  centroid.x = centroid.x / static_cast<double>(pointcloud.size());
  centroid.y = centroid.y / static_cast<double>(pointcloud.size());
  return centroid;
}

Point convertPointRosToBoost(const geometry_msgs::msg::Point &point)
{
  const Point point2d(point.x, point.y);
  return point2d;
}

geometry_msgs::msg::Vector3 rpyFromQuat(const geometry_msgs::msg::Quaternion &q)
{
  tf2::Quaternion quat(q.x, q.y, q.z, q.w);
  tf2::Matrix3x3 mat(quat);
  double roll, pitch, yaw;
  mat.getRPY(roll, pitch, yaw);
  geometry_msgs::msg::Vector3 rpy;
  rpy.x = roll;
  rpy.y = pitch;
  rpy.z = yaw;
  return rpy;
}

Polygon getPolygon(const geometry_msgs::msg::Pose &pose, const geometry_msgs::msg::Vector3 &size,
                   const double center_offset, const double l_margin = 0.0, const double w_margin = 0.0)
{
  Polygon obj_poly;
  geometry_msgs::msg::Vector3 obj_rpy = rpyFromQuat(pose.orientation);

  double l = size.x * std::cos(obj_rpy.y) + l_margin;
  double w = size.y * std::cos(obj_rpy.x) + w_margin;
  double co = center_offset;
  bg::exterior_ring(obj_poly) = boost::assign::list_of<Point>(l / 2.0 + co, w / 2.0)(-l / 2.0 + co, w / 2.0)(
      -l / 2.0 + co, -w / 2.0)(l / 2.0 + co, -w / 2.0)(l / 2.0 + co, w / 2.0);

  // rotate polygon
  bg::strategy::transform::rotate_transformer<bg::radian, double, 2, 2> rotate(-obj_rpy.z);  // original:clockwise
                                                                                             // rotation
  Polygon rotate_obj_poly;
  bg::transform(obj_poly, rotate_obj_poly, rotate);
  // translate polygon
  bg::strategy::transform::translate_transformer<double, 2, 2> translate(pose.position.x, pose.position.y);
  Polygon translate_obj_poly;
  bg::transform(rotate_obj_poly, translate_obj_poly, translate);
  return translate_obj_poly;
}

double getDistanceFromTwoPoint(const geometry_msgs::msg::Point &point1, const geometry_msgs::msg::Point &point2)
{
  const double dx = point1.x - point2.x;
  const double dy = point1.y - point2.y;
  const double dist = std::hypot(dx, dy);
  return dist;
}
}  // namespace

#endif  // OBSTACLE_STOP_PLANNER__UTIL_HPP_