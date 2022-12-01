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
    Plane(const std::vector<ORB_SLAM3::MapPoint*> &vMPs, const cv::Mat &Tcw);

    void Recompute();

    //normal
    cv::Mat n;
    //origin
    cv::Mat o;
    //arbitrary orientation along normal
    float rang;
    //transformation from world to the plane
    cv::Mat Tpw;
    glm::mat4 glTpw;
    //MapPoints that define the plane
    std::vector<ORB_SLAM3::MapPoint*> mvMPs;
    //camera pose when the plane was first observed (to compute normal direction)
    cv::Mat mTcw, XC;
};

#endif // PLANE_H