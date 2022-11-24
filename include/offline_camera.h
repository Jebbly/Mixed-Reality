#include <iostream>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class OfflineCamera
{
private:
    std::vector<std::string> m_rgb_images;
    std::vector<std::string> m_depth_images;
    std::vector<double> m_timestamps;

public:
    OfflineCamera(const std::string& associated_files);

    cv::Mat get_rgb_image(size_t index) const;
    cv::Mat get_depth_image(size_t index) const;
    double get_timestamp(size_t index) const;
};