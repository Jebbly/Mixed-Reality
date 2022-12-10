#ifndef OFFLINE_CAMERA_H
#define OFFLINE_CAMERA_H

#include <fstream>
#include <iostream>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class CameraStream
{
public:
    CameraStream();
    ~CameraStream();

    virtual std::tuple<cv::Mat, cv::Mat, double> get_stream() = 0;
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
    OfflineCameraStream(const std::string& dataset_dir);

    virtual std::tuple<cv::Mat, cv::Mat, double> get_stream();
};

#endif // CAMERA_STREAM_H