#ifndef PLANE_H
#define PLANE_H

#include <iostream>
#include <mutex>
#include <vector>

#include <GLFW/glfw3.h>
#include <glm/matrix.hpp>
#include <opencv2/core/core.hpp>
#include <MapPoint.h>

class Plane
{
public:
    Plane(const std::vector<ORB_SLAM3::MapPoint*> &plane_points, const cv::Mat &camera_pose);
    void recompute(const cv::Mat &camera_pose);

    cv::Mat normal, origin;
    float orientation;
    glm::mat4 model_matrix;
    std::vector<ORB_SLAM3::MapPoint*> map_points;
};

Plane* add_object(const std::vector<ORB_SLAM3::MapPoint*> &curr_map_points,
                  const std::vector<cv::KeyPoint> &curr_key_points,
                  const cv::Mat &curr_camera_pose);

#endif // PLANE_H