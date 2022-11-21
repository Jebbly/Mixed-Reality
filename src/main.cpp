#include <iostream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

const int width = 800;
const int height = 600;

void framebuffer_resize(GLFWwindow* window, int new_width, int new_height)
{
    glViewport(0, 0, new_width, new_height);
}

int main()
{
    // initialize and configure GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // create window and make context current
    GLFWwindow* window = glfwCreateWindow(width, height, "Mixed Reality Demo", NULL, NULL);
    if (window == NULL)
    {
        // throw exception
        std::cerr << "GLFW Error" << std::endl;
        return -1;
    }
    glfwMakeContextCurrent(window);

    // load address of OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
          std::cerr << "GLAD Error" << std::endl;
          return -1;
    }

    glViewport(0, 0, width, height);
    glfwSetFramebufferSizeCallback(window, framebuffer_resize);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    float clear_value = 1.0f;
    int counter = 0;
    while(!glfwWindowShouldClose(window))
    {
        glClearColor(clear_value, clear_value, clear_value, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Hello, world!");              
        ImGui::Text("This is some useful text.");   
        ImGui::SliderFloat("float", &clear_value, 0.0f, 1.0f);
        if (ImGui::Button("Button"))     
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
                    1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}