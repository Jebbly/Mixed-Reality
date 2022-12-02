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

#include "geometry.h"
#include "plane.h"
#include "shader_util.h"

class Renderer
{
private:
    size_t m_width, m_height;
    GLFWwindow* m_window;

    std::string m_shader_dir;

    std::mutex m_info_mutex;
    cv::Mat m_background_image;
    cv::Mat m_completed_depth;
    cv::Mat m_camera_pose;
    std::vector<ORB_SLAM3::MapPoint*> m_map_points;
    std::vector<cv::KeyPoint> m_key_points;

    GLuint m_quad_vao;
    GLuint m_background_texture;
    bool m_image_updated;
    GLuint m_background_shader;

    glm::mat4 m_persp;
    GLuint m_object_vao;
    GLuint m_object_shader;

    std::vector<Plane*> m_planes;

    bool m_draw_key_points;
    bool m_add_object;
    bool m_should_close;

    void init_gl();

    void init_ui();
    void draw_ui();

    void init_background_image();
    void draw_background_image();

    void init_objects();
    void draw_object(size_t index);
    void draw_objects();

    void draw_key_points();

public:
    Renderer(size_t width, size_t height, const std::string& shader_dir);
    ~Renderer();

    void run();
    void set_info(const cv::Mat &rgb_image, const cv::Mat &depth_image, const cv::Mat &pose, 
                  const std::vector<ORB_SLAM3::MapPoint*> &map_points,
                  const std::vector<cv::KeyPoint> &key_points);
    void set_close();
};