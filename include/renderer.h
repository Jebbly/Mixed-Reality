#ifndef RENDERER_H
#define RENDERER_H

#include <chrono>
#include <iostream>
#include <mutex>
#include <vector>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <opencv2/core/core.hpp>
#include <MapPoint.h>

#include "util/geometry_util.h"
#include "util/matrix_util.h"
#include "util/shader_util.h"
#include "light_estimation.h"

class Renderer
{
private:
    // Properties of the renderer
    size_t m_width, m_height;
    GLFWwindow* m_window;
    std::string m_shader_dir, m_model_path, m_camera_settings;

    // Info needed to render or add an object
    std::mutex m_slam_mutex;
    cv::Mat m_camera_pose;
    std::vector<ORB_SLAM3::MapPoint*> m_map_points;
    std::vector<cv::KeyPoint> m_key_points;

    std::mutex m_image_mutex;
    cv::Mat m_background_image;
    cv::Mat m_completed_depth;

    std::mutex m_light_mutex;
    std::vector<Light> m_lights;

    // Geometry pass rendering
    Scene m_scene;
    Shader m_geometry_shader;
    GLuint m_geometry_fbo;
    glm::mat4 m_persp;
    std::vector<Model> m_models;

    // Deferred pass rendering
    Shader m_deferred_shader;
    GLuint m_positions, m_normals, m_diff_spec;

    // Quad rendering objects
    Shader m_image_shader;
    GLuint m_quad_vao;
    GLuint m_background_texture, m_depth_texture;

    // Flags to control renderer behavior
    bool m_image_updated;
    bool m_draw_key_points;
    bool m_add_object;
    bool m_should_close;

    // Externally check when an object was added
    std::mutex m_object_mutex;
    Plane* m_last_object_added;

    // Timestep for animation
    std::chrono::time_point<std::chrono::system_clock> m_last_frame;

public:
    // Some things need to be initialized/destroyed before/after the main loop
    Renderer(size_t width, size_t height, const std::string &settings, const std::string &shaders, const std::string &model_path);
    ~Renderer();

    // Main event loop and mark when to close
    void run();
    void close();

    // Pass info from another thread to the renderer thread
    void set_slam(const cv::Mat &pose, 
                  const std::vector<ORB_SLAM3::MapPoint*> &map_points,
                  const std::vector<cv::KeyPoint> &key_points);

    void set_images(const cv::Mat &rgb_image, const cv::Mat &depth_image);

    void set_lights(const std::vector<Light> &lights);

    void add_object(const cv::Mat &origin, const cv::Mat &normal, float orientation);
    
    // The main loop needs to know at which point an object was added
    Plane* get_most_recent_object();

private:
    // Initialization helpers
    void init_window();
    void init_gl();
    void init_framebuffer();
    void init_shaders();
    void init_images();
    void init_scene();
    void init_ui();

    // Renderer drawing helpers
    void draw_key_points();
    void draw_background_image();
    void draw_scene();
    void draw_ui();
};

#endif // RENDERER_H