#include "offline_camera.h"

OfflineCamera::OfflineCamera(const std::string& dataset_dir) :
    m_dataset_dir{dataset_dir}
{
    std::string associated_files = dataset_dir + "/associated.txt";

    std::cout << "[OFFLINE CAMERA]: Reading associations from " << associated_files << std::endl;
    std::ifstream associations(associated_files.c_str());
    if (!associations.is_open()) {
        std::cerr << "[OFFLINE CAMERA]: Can't open associated.txt file!" << std::endl;
    }

    std::string line;
    while (std::getline(associations, line))
    {
        if(!line.empty())
        {
            std::stringstream ss;
            ss << line;

            double t;
            std::string rgb_image, depth_image;

            ss >> t;
            ss >> rgb_image;
            ss >> t;
            ss >> depth_image;

            m_timestamps.push_back(t);
            m_rgb_images.push_back(rgb_image);
            m_depth_images.push_back(depth_image);
        }
    }

    std::cout << "[OFFLINE CAMERA]: Read " << m_rgb_images.size() << " dataset images" << std::endl;
}

cv::Mat OfflineCamera::get_rgb_image(size_t index) const
{
    return cv::imread(m_dataset_dir + "/" + m_rgb_images[index], cv::IMREAD_UNCHANGED);
}

cv::Mat OfflineCamera::get_depth_image(size_t index) const
{
    return cv::imread(m_dataset_dir + "/" + m_depth_images[index], cv::IMREAD_UNCHANGED);
}

double OfflineCamera::get_timestamp(size_t index) const
{
    return m_timestamps[index];
}

int OfflineCamera::get_total_frames() const
{
    return m_rgb_images.size();
}