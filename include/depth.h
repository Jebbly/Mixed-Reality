#ifndef DEPTH_H
#define DEPTH_H

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <vector>

#include<opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class OfflineDepthCompleter
{
private:
    std::string m_dataset_dir;
    std::vector<std::string> m_depth_images;
    int m_image_idx;

    cv::Mat m_completed_depth;

public:
    OfflineDepthCompleter(const std::string &dataset_dir);

    void complete_depth_image(const cv::Mat &incomplete_depth_image);
    const cv::Mat& get_depth_image();
};

#endif // DEPTH_H