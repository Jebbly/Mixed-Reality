#include <iostream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader_util.h"

const float vertices[] = {-1.0f, -1.0f, 
                          -1.0f,  1.0f, 
                           1.0f,  1.0f, 
                           1.0f, -1.0f};

const float texcoords[] = {0.0f, 0.0f, 
                           0.0f, 1.0f, 
                           1.0f, 1.0f, 
                           1.0f, 0.0f};

const int indices[] = {0, 1, 2,
                       0, 2, 3};

class Renderer
{
private:
    size_t m_width, m_height;
    GLFWwindow* m_window;

    std::string m_shader_dir;

    GLuint m_quad_vao;
    GLuint m_background_image;
    bool m_image_updated;
    GLuint m_background_shader;

    void init_ui();
    void draw_ui();

    void init_background_image();
    void draw_background_image();

public:
    Renderer(size_t width, size_t height, const std::string& shader_dir);
    ~Renderer();

    void run();
};