#include "util/camera_util.h"

std::vector<std::tuple<std::string, std::string, double>> load_offline_dataset(const std::string &dataset_dir, OfflineDatasetType type)
{
    std::string associated_files = dataset_dir;
    switch (type) {
        case OfflineDatasetType::ETH3D: {
            associated_files += "/associated.txt";
            break;
        }
        case OfflineDatasetType::SCANNET: {
            associated_files += "/poses/groundtruth.txt";
            break;
        }
    }

    std::cout << "[OFFLINE CAMERA]: Reading associations from " << associated_files << std::endl;
    std::ifstream associations(associated_files);
    if (!associations.is_open()) {
        std::cerr << "[OFFLINE CAMERA]: Can't open associated.txt file!" << std::endl;
    }

    // Each tuple represents the RGB image, depth image, and timestamp of a frame
    std::vector<std::tuple<std::string, std::string, double>> ret;

    std::string line;
    while (std::getline(associations, line)) {
        if(!line.empty()) {
            switch (type) {
                case OfflineDatasetType::ETH3D: {
                    ret.push_back(process_eth3d(line));
                    break;
                }
                case OfflineDatasetType::SCANNET: {
                    ret.push_back(process_scannet(line));
                    break;
                }
            }
        }
    }

    return ret;
}

std::tuple<std::string, std::string, double> process_eth3d(const std::string &line)
{
    std::stringstream ss;
    ss << line;

    double t;
    std::string rgb_image, depth_image;

    ss >> t;    
    ss >> rgb_image;
    ss >> t;
    ss >> depth_image;

    return std::make_tuple(rgb_image, depth_image, t);
}

std::tuple<std::string, std::string, double> process_scannet(const std::string &line)
{
    std::stringstream ss;
    ss << line;

    double t, pose_info;
    std::string rgb_image, depth_image;

    ss >> t;
    // The associated file also provides unnecessary information
    for (int i = 0; i < 7; i++) {
        ss >> pose_info;
    }
    ss >> t;
    ss >> depth_image;
    ss >> t;
    ss >> rgb_image;

    return std::make_tuple(rgb_image, depth_image, t);
}