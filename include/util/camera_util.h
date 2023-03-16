#ifndef CAMERA_UTIL_H
#define CAMERA_UTIL_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <tuple> 
#include <vector>

enum class OfflineDatasetType
{
    ETH3D,
    SCANNET,
};

std::vector<std::tuple<std::string, std::string, double>> load_offline_dataset(const std::string &dataset_dir, OfflineDatasetType type);

std::tuple<std::string, std::string, double> process_eth3d(const std::string &line);
std::tuple<std::string, std::string, double> process_scannet(const std::string &line);

#endif // CAMERA_UTIL_H