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
OfflineDepthCompleter::OfflineDepthCompleter(const std::string &dataset_dir) :
    DepthCompleter{},
    m_dataset_dir{dataset_dir},
    m_image_idx{0}
{
    // We use the same association files as in camera_stream.cpp, but modify it to read the final depth images
    std::string associated_files = dataset_dir + "/associated.txt";

    std::cout << "[OFFLINE DEPTH COMPLETER]: Reading associations from " << associated_files << std::endl;
    std::ifstream associations(associated_files.c_str());
    if (!associations.is_open()) {
        std::cerr << "[OFFLINE DEPTH COMPLETER]: Can't open associated.txt file!" << std::endl;
    }

    std::string line;
    int index = 0;
    while (std::getline(associations, line))
    {
        if (index < 2) {
            index++;
            continue;
        }

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
            m_depth_images.push_back("final_raycast_" + depth_image);
        }
        index++;
    }

    std::cout << "[OFFLINE DEPTH COMPLETER]: Read " << m_depth_images.size() << " depth images" << std::endl;
}

void OfflineDepthCompleter::complete_depth_image(const cv::Mat &incomplete_depth_image)
{
    if (m_image_idx >= 2) {
        // This implementation requires 2 frames to initialize
        cv::Mat raw_depth = cv::imread(m_dataset_dir + "/" + m_depth_images[m_image_idx - 2], cv::IMREAD_UNCHANGED);
        cv::Mat buffer = raw_depth;
        raw_depth.convertTo(buffer, CV_32F);
        m_completed_depth = buffer / 5000.0f;
    } 
    m_image_idx++;
}