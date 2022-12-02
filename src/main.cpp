#include <chrono>
#include <iostream>
#include <thread>
#include "renderer.h"
#include "offline_camera.h"

#include "System.h"

// The window dimensions are slightly different
// from the actual image dimensions because
// there are byte alignment requirements
// when we copy the image data to OpenGL.
const int width = 736;
const int height = 456;

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

    ORB_SLAM3::System SLAM(argv[1],argv[2],ORB_SLAM3::System::RGBD,false);
    OfflineCamera camera{std::string(argv[3])};
    Renderer renderer(width, height, std::string(argv[4]));
    
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

        // // can handle this better
        cv::resize(rgb_image, rgb_image, cv::Size(width, height));
        cv::resize(depth_image, depth_image, cv::Size(width, height));
        if(image_scale != 1.0f)
        {
            int new_width = rgb_image.cols * image_scale;
            int new_height = rgb_image.rows * image_scale;
            cv::resize(rgb_image, rgb_image, cv::Size(new_width, new_height));
            cv::resize(depth_image, depth_image, cv::Size(new_width, new_height));
        }

        cv::Mat camera_pose = ORB_SLAM3::Converter::toCvMat(SLAM.TrackRGBD(rgb_image, depth_image, timestamp).matrix());
        int state = SLAM.GetTrackingState();
		std::vector<ORB_SLAM3::MapPoint*> vMPs = SLAM.GetTrackedMapPoints();
		std::vector<cv::KeyPoint> vKeys = SLAM.GetTrackedKeyPointsUn();

        // This depth image isn't actually the correct one;
        // We want the completed one instead.
        renderer.set_info(rgb_image, depth_image, camera_pose, vMPs, vKeys);
        
        // This controls the offline camera's speed 
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    renderer.set_close();
    thread.join();

    return 0;
}