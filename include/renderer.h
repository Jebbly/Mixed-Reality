#include <iostream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Renderer
{
private:
    GLFWwindow* m_window;
    float m_clear_color;

    void draw_ui();

public:
    Renderer(size_t width, size_t height);
    ~Renderer();

    void run();
};