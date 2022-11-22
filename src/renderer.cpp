#include "renderer.h"

void framebuffer_resize(GLFWwindow* window, int new_width, int new_height)
{
    glViewport(0, 0, new_width, new_height);
}

void Renderer::draw_ui()
{
    static int counter = 0;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Hello, world!");              
    ImGui::Text("This is some useful text.");   
    ImGui::SliderFloat("float", &m_clear_color, 0.0f, 1.0f);
    if (ImGui::Button("Button"))     
        counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

Renderer::Renderer(size_t width, size_t height)
{
    // initialize and configure GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // create window and make context current
    m_window = glfwCreateWindow(width, height, "Mixed Reality Demo", NULL, NULL);
    if (m_window == NULL)
    {
        // throw exception
        std::cerr << "GLFW Error" << std::endl;
    }
    glfwMakeContextCurrent(m_window);

    // load address of OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
          std::cerr << "GLAD Error" << std::endl;
    }

    glViewport(0, 0, width, height);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_resize);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

Renderer::~Renderer()
{
    glfwTerminate();
}

void Renderer::run()
{
    while (!glfwWindowShouldClose(m_window))
    {
        glClearColor(m_clear_color, m_clear_color, m_clear_color, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        draw_ui();

        glfwPollEvents();
        glfwSwapBuffers(m_window);
    }
}