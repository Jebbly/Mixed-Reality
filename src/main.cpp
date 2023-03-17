#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream> 
#include <string>
#include <thread>

#include <System.h> // ORB-SLAM system needed for tracking

#include "renderer.h"
#include "camera_stream.h"
#include "depth_completion.h"
#include "light_estimation.h"

const int NUM_LIGHTS = 4;

std::vector<std::tuple<int, cv::Mat, cv::Mat, float>> read_recording(const std::string &filepath) 
{
    std::ifstream record_file (filepath);
    std::vector<std::tuple<int, cv::Mat, cv::Mat, float>> ret;

    if (!record_file.is_open()) {
        return ret;
    }

    std::cout << "[MAIN LOOP]: Reading records from " << filepath << std::endl;

    std::string line;
     while (std::getline(record_file, line)) {
        if(!line.empty()) {
            std::stringstream ss;
            ss << line;

            int index;
            float o_x, o_y, o_z, n_x, n_y, n_z, orientation;
            ss >> index >> o_x >> o_y >> o_z >> n_x >> n_y >> n_z >> orientation;

            cv::Mat origin = (cv::Mat_<float>(3, 1) << o_x, o_y, o_z);
            cv::Mat normal = (cv::Mat_<float>(3, 1) << n_x, n_y, n_z);
            ret.push_back(std::make_tuple(index, origin, normal, orientation));
        }
    }

    return ret;
}

void write_recording(const std::string &filepath, const std::vector<std::tuple<int, cv::Mat, cv::Mat, float>> &recordings)
{
    std::ofstream record_file(filepath);

    for (int i = 0; i < recordings.size(); i++) {
        const std::tuple<int, cv::Mat, cv::Mat, float> &recording = recordings[i];

        int index = std::get<0>(recording);
        const cv::Mat &origin = std::get<1>(recording);
        const cv::Mat &normal = std::get<2>(recording);
        float orientation = std::get<3>(recording);

        record_file << index << " " <<
                       origin.at<float>(0, 0) << " " << origin.at<float>(0, 1) << " " << origin.at<float>(0, 2) << " " <<
                       normal.at<float>(0, 0) << " " << normal.at<float>(0, 1) << " " << normal.at<float>(0, 2) << " " <<
                       orientation << std::endl;
    }  

    record_file.close();
}

int main(int argc, char* argv[])
{
    if (argc < 6) {
        std::cerr << "Usage: ./mixed_reality [vocabulary_file] [settings_file] [shader_dir] [model_file] [dataset_dir] [(optional) record_file]" << std::endl;
        // Optional argument at the end: filepath to record when the objects were placed
        return -1;
    }

    // Parse the dataset type from the given arguments
    std::string settings = argv[2];
    OfflineDatasetType type;
    if (settings.find("ETH3D") != std::string::npos) {
        type = OfflineDatasetType::ETH3D;
    } else if (settings.find("ScanNet") != std::string::npos) {
        type = OfflineDatasetType::SCANNET;
    } else {
        std::cerr << "Invalid dataset type provided" << std::endl;
        return -1;
    }

    // When optional filepath is provided, open the file and check if there is already a recording.
    // If there is, then read from the recording, otherwise we write to the recording.
    std::vector<std::tuple<int, cv::Mat, cv::Mat, float>> recordings;
    bool record_file_exists = false;
    bool read_or_write = false;
    if (argc > 6) {
        record_file_exists = true;
        recordings = read_recording(argv[6]);
        read_or_write = (recordings.size() > 0);
    }

    // Camera implementation
    CameraStream* camera = new OfflineCameraStream(argv[5], type);

    // The window dimensions are slightly different
    // from the actual image dimensions because
    // there are byte alignment requirements
    // when we copy the image data to OpenGL.
    int width = camera->get_width(), height = camera->get_height();

    // Start the SLAM and renderer threads
    ORB_SLAM3::System SLAM(argv[1], argv[2], ORB_SLAM3::System::RGBD, false);
    Renderer renderer(width, height, argv[2], argv[3], argv[4]);
    std::thread thread = std::thread(&Renderer::run, &renderer);

    // Arbitrary time for the renderer to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds(16));

    // Implementations of light source estimation and depth completion
    LightEstimator* light_estimator = new RandLightEstimator(NUM_LIGHTS);
    DepthCompleter* depth_completer = new OfflineDepthCompleter(argv[5], "", type);

    // The completed depths have 4 fewer frames than the dataset,
    // so we have to adjust for the indexing.
    int record_idx = 0;
    int num_frames = camera->get_frame_count();
    for (int i = 0; i < num_frames; i++) {
        std::tuple<cv::Mat, cv::Mat, double> stream = camera->get_stream();

        cv::Mat rgb_image, depth_image;
        cv::resize(std::get<0>(stream), rgb_image, cv::Size(width, height));
        cv::resize(std::get<1>(stream), depth_image, cv::Size(width, height));
        double timestamp = std::get<2>(stream);

        // We always want to update the pose whenever we update the image
        cv::Mat camera_pose = ORB_SLAM3::Converter::toCvMat(SLAM.TrackRGBD(rgb_image, depth_image, timestamp).matrix());
        int state = SLAM.GetTrackingState();
		std::vector<ORB_SLAM3::MapPoint*> map_points = SLAM.GetTrackedMapPoints();
		std::vector<cv::KeyPoint> key_points = SLAM.GetTrackedKeyPointsUn();

        // Estimate lights and complete depth
        const std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

        light_estimator->estimate_lights(rgb_image, depth_image);
        std::vector<Light> lights = light_estimator->get_lights();

        depth_completer->complete_depth_image(depth_image);
        cv::Mat completed_depth = depth_completer->get_depth_image();

        // If either algorithm isn't available yet, skip the frame
        if (lights.empty() || completed_depth.empty()) {
            continue;
        }
        cv::resize(completed_depth, completed_depth, cv::Size(width, height));

        const std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();

        // If the depth completion and light estimation took longer than 17 ms (~60 FPS), log it
        int milliseconds_passed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        if (milliseconds_passed > 17) {
            std::cout << "[MAIN LOOP]: Missed 17 ms deadline for depth completion and light estimation by " << milliseconds_passed - 17 << " milliseconds" << std::endl; 
        }

        // When everything is available, we pass the information to the renderer.
        renderer.set_slam(camera_pose, map_points, key_points);
        renderer.set_images(rgb_image, completed_depth);
        renderer.set_lights(lights);

        // If we're reading from a recording, check if we're at an object.
        // Otherwise if we're recording, check if an object was added.
        if (record_file_exists) {
            if (read_or_write) {
                std::tuple<int, cv::Mat, cv::Mat, float> &record = recordings[record_idx];

                // If we're at a frame where an object was recorded, add it.
                if (std::get<0>(record) == i) {
                    renderer.add_object(std::get<1>(record), std::get<2>(record), std::get<3>(record));
                    std::cout << "[MAIN LOOP]: Adding recorded object at frame " << i << std::endl;
                    record_idx++;
                }
            } else {
                Plane* object_added = renderer.get_most_recent_object();
                if (object_added) {
                    std::tuple<cv::Mat, cv::Mat, float> info = object_added->get_plane_information();
                    recordings.push_back(std::make_tuple(i, std::get<0>(info), std::get<1>(info), std::get<2>(info)));
                    std::cout << "[MAIN LOOP]: Recording object added at frame " << i << std::endl;
                }
            }
        }

        // This controls the offline camera's speed 
        std::this_thread::sleep_for(std::chrono::milliseconds(17 - milliseconds_passed));
    }

    renderer.close();
    thread.join();

    // If we recorded objects, write out to the file
    if (record_file_exists && !read_or_write) {
        write_recording(argv[6], recordings);
    }

    delete camera;
    delete light_estimator;
    delete depth_completer;

    return 0;
}