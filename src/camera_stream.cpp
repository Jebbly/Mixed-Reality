#include "camera_stream.h"

// Interface definition
CameraStream::CameraStream()
{

}

CameraStream::~CameraStream()
{
    
}

int CameraStream::get_width() const
{
    return m_width;
}

int CameraStream::get_height() const
{
    return m_height;
}

int CameraStream::get_frame_count() const
{
    return m_frame_count;
}

// Implementation definitions
OfflineCameraStream::OfflineCameraStream(const std::string& dataset_dir, OfflineDatasetType type) :
    CameraStream{},
    m_dataset_dir{dataset_dir},
    m_index{0}
{
    std::vector<std::tuple<std::string, std::string, double>> dataset = load_offline_dataset(dataset_dir, type);

    for (int i = 0; i < dataset.size(); i++) {
        std::tuple<std::string, std::string, double> &frame = dataset[i];
        m_rgb_images.push_back(std::get<0>(frame));
        m_depth_images.push_back(std::get<1>(frame));
        m_timestamps.push_back(std::get<2>(frame));
    }

    m_frame_count = m_rgb_images.size();

    if (m_frame_count == 0) {
        throw std::runtime_error("[OFFLINE CAMERA]: No RGB images loaded");
    }

    switch (type) {
        case OfflineDatasetType::ETH3D: {
            m_width = 736;
            m_height = 456;
            break;
        }
        case OfflineDatasetType::SCANNET: {
            m_width = 1296;
            m_height = 968;
            break;
        }
    }

    std::cout << "[OFFLINE CAMERA]: Read " << m_frame_count << " dataset images" << std::endl;
}

std::tuple<cv::Mat, cv::Mat, double> OfflineCameraStream::get_stream()
{
    cv::Mat rgb = cv::imread(m_dataset_dir + "/" + m_rgb_images[m_index], cv::IMREAD_UNCHANGED);
    cv::Mat depth = cv::imread(m_dataset_dir + "/" + m_depth_images[m_index], cv::IMREAD_UNCHANGED);
    double timestamp = m_timestamps[m_index];
    m_index++;

    return std::tuple<cv::Mat, cv::Mat, double>(rgb, depth, timestamp);
}