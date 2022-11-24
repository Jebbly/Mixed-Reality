#include "renderer.h"

void framebuffer_resize(GLFWwindow* window, int new_width, int new_height)
{
    glViewport(0, 0, new_width, new_height);
}

void Renderer::init_ui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    std::cout << "UI initialized" << std::endl;
}

void Renderer::draw_ui()
{
    // Setup the ImGui context
    static int counter = 0;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Actions");   

    // Temporary filler action
    if (ImGui::Button("Insert Cube"))     
        counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    // Also display application statistics
    ImGui::NewLine();
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Renderer::init_background_image()
{
    // Configure a texture in OpenGL to store the background image
    glGenTextures(1, &m_background_image);
    glBindTexture(GL_TEXTURE_2D, m_background_image);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_LUMINANCE, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    // Draw a screen quad to sample the background image
    glGenVertexArrays(1, &m_quad_vao);
    glBindVertexArray(m_quad_vao);

    GLuint vertex_buffer = 0;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    GLuint texcoord_buffer = 0;
    glGenBuffers(1, &texcoord_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, texcoord_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(1);

    GLuint index_buffer = 0;
    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // We also need a shader to only draw the background image
    m_background_shader = create_program(m_shader_dir + "/vertex.glsl", m_shader_dir + "/fragment.glsl");

    // Set the flag to indicate that there is new data
    m_image_updated = true;

    std::cout << "Background image shader initialized" << std::endl;
}

void Renderer::draw_background_image() 
{
    // need a mutex here
    // Get the most recently updated image
    // if the image has changed
    glUseProgram(m_background_shader);
    glBindVertexArray(m_quad_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

Renderer::Renderer(size_t width, size_t height, const std::string& shader_dir) : m_width{width}, m_height{height}, m_shader_dir{shader_dir}
{
    // initialize and configure GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // create window and make context current
    m_window = glfwCreateWindow(m_width, m_height, "Mixed Reality Demo", NULL, NULL);
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

    glViewport(0, 0, m_width, m_height);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_resize);

    init_ui();
    init_background_image();
}

Renderer::~Renderer()
{
    glfwTerminate();
}

void Renderer::run()
{
    while (!glfwWindowShouldClose(m_window))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        draw_background_image();

        // draw the UI on top of everything else
        draw_ui();

        glfwPollEvents();
        glfwSwapBuffers(m_window);
    }
}