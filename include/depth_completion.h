#ifndef DEPTH_COMPLETION_H
#define DEPTH_COMPLETION_H

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <vector>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "util/camera_util.h"

// Base class defines an interface for depth completion
class DepthCompleter
{
protected:
    cv::Mat m_completed_depth;

public:
    DepthCompleter();
    virtual ~DepthCompleter();

    virtual void complete_depth_image(const cv::Mat &incomplete_depth_image) = 0;
    const cv::Mat& get_depth_image() const;
};

// This implementation looks up pre-computed completed depths.
class OfflineDepthCompleter : public DepthCompleter
{
private:
    std::string m_dataset_dir;
    std::vector<std::string> m_depth_images;
    int m_image_idx;

public:
    OfflineDepthCompleter(const std::string &dataset_dir, OfflineDatasetType type);

    virtual void complete_depth_image(const cv::Mat &incomplete_depth_image);
};

#endif // DEPTH_COMPLETION_H