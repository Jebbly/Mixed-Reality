#ifndef RENDERER_H
#define RENDERER_H

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

#include "util/geometry.h"
#include "util/matrix_util.h"
#include "util/shader_util.h"
#include "plane.h"
#include "light_estimation.h"

class Renderer
{
private:
    // Properties of the renderer
    size_t m_width, m_height;
    GLFWwindow* m_window;
    std::string m_shader_dir, m_camera_settings;

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

    // OpenGL objects needed for rendering
    GLuint m_image_shader, m_geometry_shader, m_deferred_shader;
    GLuint m_geometry_fbo;
    GLuint m_positions, m_normals;
    GLuint m_quad_vao, m_geometry_vao;
    GLuint m_background_texture, m_depth_texture;
    glm::mat4 m_persp;
    std::vector<Plane*> m_planes;

    // Flags to control renderer behavior
    bool m_image_updated;
    bool m_draw_key_points;
    bool m_add_object;
    bool m_should_close;

public:
    // Some things need to be initialized/destroyed before/after the main loop
    Renderer(size_t width, size_t height, const std::string &settings, const std::string &shaders);
    ~Renderer();

    // Main event loop and mark when to close
    void run();
    void close();

    // Pass info from another thread to the renderer thread
    void set_slam(const cv::Mat &pose, 
                  const std::vector<ORB_SLAM3::MapPoint*> &map_points,
                  const std::vector<cv::KeyPoint> &key_points);

    void set_images(const cv::Mat& rgb_image, const cv::Mat &depth_image);

    void set_lights(const std::vector<Light> &lights);

private:
    // Initialization helpers
    void init_window();
    void init_gl();
    void init_framebuffer();
    void init_shaders();
    void init_images();
    void init_objects();
    void init_ui();

    // Renderer drawing helpers
    void draw_key_points();
    void draw_background_image();
    void draw_objects();
    void draw_ui();
};

#endif // RENDERER_H