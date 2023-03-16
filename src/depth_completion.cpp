#include "depth_completion.h"

// Interface definition
DepthCompleter::DepthCompleter()
{

}

DepthCompleter::~DepthCompleter()
{
    
}

const cv::Mat& DepthCompleter::get_depth_image() const
{
    return m_completed_depth;
}

// Implementation definitions
OfflineDepthCompleter::OfflineDepthCompleter(const std::string &dataset_dir, OfflineDatasetType type) :
    DepthCompleter{},
    m_dataset_dir{dataset_dir},
    m_image_idx{0}
{
    std::vector<std::tuple<std::string, std::string, double>> dataset = load_offline_dataset(dataset_dir, type);

     for (int i = 0; i < dataset.size(); i++) {
        std::tuple<std::string, std::string, double> &frame = dataset[i];
        
        // We only care about the depth image in this case
        m_depth_images.push_back("final_raycast_" + std::get<1>(frame));
    }

    std::cout << "[OFFLINE DEPTH COMPLETER]: Read " << m_depth_images.size() << " depth images" << std::endl;
}

void OfflineDepthCompleter::complete_depth_image(const cv::Mat &incomplete_depth_image)
{
    // This implementation requires 2 frames to initialize
    cv::Mat raw_depth = cv::imread(m_dataset_dir + "/" + m_depth_images[m_image_idx], cv::IMREAD_UNCHANGED);
    cv::Mat buffer = raw_depth;
    raw_depth.convertTo(buffer, CV_32F);

    // Replace 0 values with a high value (arbitrarily set to 10.0f)
    cv::Mat mask = (buffer == 0);
    cv::Mat mask_buffer = mask;
    mask.convertTo(mask_buffer, CV_32F);

    m_completed_depth = 10.0f * mask_buffer + buffer / 5000.0f;

    m_image_idx++;
}
