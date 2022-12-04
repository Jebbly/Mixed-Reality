#include <chrono>
#include <iostream>
#include <thread>
#include "renderer.h"
#include "offline_camera.h"
#include "depth.h"
#include "light.h"

#include "System.h"

// The window dimensions are slightly different
// from the actual image dimensions because
// there are byte alignment requirements
// when we copy the image data to OpenGL.
const int WIDTH = 736;
const int HEIGHT = 456;

const int NUM_LIGHTS = 4;

int main(int argc, char* argv[])
{
    if(argc < 5)
    {
        /*
        ./mixed_reality /home/jebbly/Desktop/Mixed-Reality/ORB-SLAM/Vocabulary/ORBvoc.txt /home/jebbly/Desktop/Mixed-Reality/Mixed-Reality/config/ETH3D_resize.yaml /home/jebbly/Desktop/Mixed-Reality/eth3d_table-3 /home/jebbly/Desktop/Mixed-Reality/Mixed-Reality/shaders
        */
        std::cerr << "Usage: ./mixed_reality [vocabulary_file] [settings_file] [dataset_directory] [shader_directory]" << std::endl;
        return -1;
    }

    ORB_SLAM3::System SLAM(argv[1], argv[2], ORB_SLAM3::System::RGBD, false);
    OfflineCamera camera(argv[3]);
    RandLightEstimator light_estimator(NUM_LIGHTS);
    OfflineDepthCompleter depth_completer(argv[3]);
    Renderer renderer(WIDTH, HEIGHT, argv[2], argv[4]);
    
    // Wait a little before we start processing the camera frames,
    // so that the renderer doesn't miss any of the frames.
    std::thread thread = std::thread(&Renderer::run, &renderer);

    float image_scale = SLAM.GetImageScale();
    cv::Mat rgb_image, depth_image;
    double timestamp;
    for (int i = 0; i < camera.get_total_frames(); i++) {
        rgb_image = camera.get_rgb_image(i);
        depth_image = camera.get_depth_image(i);
        timestamp = camera.get_timestamp(i);

        cv::resize(rgb_image, rgb_image, cv::Size(WIDTH, HEIGHT));
        cv::resize(depth_image, depth_image, cv::Size(WIDTH, HEIGHT));

        // We always want to update the pose whenever we update the image
        cv::Mat camera_pose = ORB_SLAM3::Converter::toCvMat(SLAM.TrackRGBD(rgb_image, depth_image, timestamp).matrix());
        int state = SLAM.GetTrackingState();
		std::vector<ORB_SLAM3::MapPoint*> map_points = SLAM.GetTrackedMapPoints();
		std::vector<cv::KeyPoint> key_points = SLAM.GetTrackedKeyPointsUn();

        // In this demo, we only estimate the light source once.
        if (i == 0)
        {
            light_estimator.estimate_lights(rgb_image, depth_image);
        }
        std::vector<Light> lights = light_estimator.get_lights();

        depth_completer.complete_depth_image(depth_image);
        cv::Mat completed_depth = depth_completer.get_depth_image();
        cv::resize(completed_depth, completed_depth, cv::Size(WIDTH, HEIGHT));

        // This depth image isn't actually the correct one;
        // We want the completed one instead.
        renderer.set_slam(camera_pose, map_points, key_points);
        renderer.set_images(rgb_image, completed_depth);
        renderer.set_lights(lights);
        
        // This controls the offline camera's speed 
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    renderer.close();
    thread.join();

    return 0;
}