#include <fstream>
#include <iostream>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class OfflineCamera
{
private:
    std::string m_dataset_dir;
    std::vector<std::string> m_rgb_images;
    std::vector<std::string> m_depth_images;
    std::vector<double> m_timestamps;

public:
    OfflineCamera(const std::string& dataset_dir);

    cv::Mat get_rgb_image(size_t index, bool show) const;
    cv::Mat get_depth_image(size_t index) const;
    double get_timestamp(size_t index) const;
    int get_total_frames() const;
};