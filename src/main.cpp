#include <chrono>
#include <iostream>
#include <thread>
#include "renderer.h"
#include "offline_camera.h"

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
        std::cerr << "Usage: ./mixed_reality [vocabulary_file] [settings_file] [dataset_directory] [shader_directory]" << std::endl;
        return -1;
    }

    OfflineCamera camera{std::string(argv[3])};
    Renderer renderer(width, height, std::string(argv[4]));
    std::thread thread = std::thread(&Renderer::run, &renderer);
    std::this_thread::sleep_for(std::chrono::milliseconds(16));

    cv::Mat rgb_image, depth_image;
    for (int i = 0; i < camera.get_total_frames(); i++) {
        rgb_image = camera.get_rgb_image(i);
        cv::resize(rgb_image, rgb_image, cv::Size(width, height));
        renderer.set_background_image(rgb_image);
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    renderer.set_close();
    thread.join();

    return 0;
}