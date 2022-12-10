#include "camera_stream.h"

// Interface definition
CameraStream::CameraStream()
{

}

CameraStream::~CameraStream()
{
    
}

// Implementation definitions
OfflineCameraStream::OfflineCameraStream(const std::string& dataset_dir) :
    CameraStream{},
    m_dataset_dir{dataset_dir},
    m_index{0}
{
    // The association files lists out the name of the images used (with timestamps)
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

std::tuple<cv::Mat, cv::Mat, double> OfflineCameraStream::get_stream()
{
    cv::Mat rgb = cv::imread(m_dataset_dir + "/" + m_rgb_images[m_index], cv::IMREAD_UNCHANGED);
    cv::Mat depth = cv::imread(m_dataset_dir + "/" + m_depth_images[m_index], cv::IMREAD_UNCHANGED);
    double timestamp = m_timestamps[m_index];
    m_index++;

    return std::tuple<cv::Mat, cv::Mat, double>(rgb, depth, timestamp);
}