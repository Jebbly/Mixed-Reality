#ifndef OFFLINE_CAMERA_H
#define OFFLINE_CAMERA_H

#include <fstream>
#include <iostream>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "util/camera_util.h"

class CameraStream
{
protected:
    int m_width, m_height;
    int m_frame_count;

public:
    CameraStream();
    ~CameraStream();

    // The camera stream is expected to return
    // the RGB image, depth image, and timestamp
    virtual std::tuple<cv::Mat, cv::Mat, double> get_stream() = 0;

    // The camera stream also provides information about
    // the camera resolution and number of frames.
    int get_width() const;
    int get_height() const;
    int get_frame_count() const;
};

class OfflineCameraStream : public CameraStream
{
private:
    std::string m_dataset_dir;
    std::vector<std::string> m_rgb_images;
    std::vector<std::string> m_depth_images;
    std::vector<double> m_timestamps;
    int m_index;

public:
    OfflineCameraStream(const std::string& dataset_dir, OfflineDatasetType type);

    virtual std::tuple<cv::Mat, cv::Mat, double> get_stream();
};

#endif // CAMERA_STREAM_H