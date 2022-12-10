#include <chrono>
#include <iostream>
#include <thread>
#include "renderer.h"
#include "camera_stream.h"
#include "depth_completion.h"
#include "light_estimation.h"

#include "System.h"

// The window dimensions are slightly different
// from the actual image dimensions because
// there are byte alignment requirements
// when we copy the image data to OpenGL.
const int WIDTH = 736;
const int HEIGHT = 456;

const int NUM_FRAMES = 1180;
const int NUM_LIGHTS = 4;

int main(int argc, char* argv[])
{
    if(argc < 5)
    {
        std::cerr << "Usage: ./mixed_reality [vocabulary_file] [settings_file] [dataset_directory] [shader_directory]" << std::endl;
        return -1;
    }

    ORB_SLAM3::System SLAM(argv[1], argv[2], ORB_SLAM3::System::RGBD, false);
    Renderer renderer(WIDTH, HEIGHT, argv[2], argv[4]);
    std::thread thread = std::thread(&Renderer::run, &renderer);
    std::this_thread::sleep_for(std::chrono::milliseconds(16));

    // Implementations of light source estimation and depth completion
    CameraStream* camera = new OfflineCameraStream(argv[3]);
    LightEstimator* light_estimator = new RandLightEstimator(NUM_LIGHTS);
    DepthCompleter* depth_completer = new OfflineDepthCompleter(argv[3]);

    // The completed depths have 4 fewer frames than the dataset,
    // so we have to adjust for the indexing.
    for (int i = 0; i < NUM_FRAMES; i++) {
        std::tuple<cv::Mat, cv::Mat, double> stream = camera->get_stream();

        cv::Mat rgb_image, depth_image;
        cv::resize(std::get<0>(stream), rgb_image, cv::Size(WIDTH, HEIGHT));
        cv::resize(std::get<1>(stream), depth_image, cv::Size(WIDTH, HEIGHT));
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
        cv::resize(completed_depth, completed_depth, cv::Size(WIDTH, HEIGHT));

        const std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();

        int milliseconds_passed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        if (milliseconds_passed > 17) {
            std::cout << "[MAIN LOOP]: Missed 17 ms deadline for depth completion and light estimation by " << milliseconds_passed - 17 << " milliseconds" << std::endl; 
        }

        // When everything is available, we pass the information to the renderer.
        renderer.set_slam(camera_pose, map_points, key_points);
        renderer.set_images(rgb_image, completed_depth);
        renderer.set_lights(lights);

        // This controls the offline camera's speed 
        std::this_thread::sleep_for(std::chrono::milliseconds(17 - milliseconds_passed));
    }

    renderer.close();
    thread.join();

    delete camera;
    delete light_estimator;
    delete depth_completer;

    return 0;
}