#include "renderer.h"

void framebuffer_resize(GLFWwindow* window, int new_width, int new_height)
{
    glViewport(0, 0, new_width, new_height);
}

void Renderer::init_gl()
{
    glfwMakeContextCurrent(m_window);

    // load address of OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
          std::cerr << "[RENDERER]: GLAD Error" << std::endl;
    }

    glViewport(0, 0, m_width, m_height);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_resize);
}

void Renderer::init_ui()
{
    // Setup the ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    std::cout << "[RENDERER]: UI initialized" << std::endl;
}

void Renderer::draw_ui()
{
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
    glGenTextures(1, &m_background_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_background_texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    std::vector<GLubyte> empty(3 * m_width * m_height, 255);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, empty.data());

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

    // Set the flag to indicate that there's no new data for now
    m_image_updated = false;

    std::cout << "[RENDERER]: Background image shader initialized" << std::endl;
}

void Renderer::draw_background_image() 
{
    // need a mutex here
    // Get the most recently updated image
    // if the image has changed
    std::lock_guard<std::mutex> lock(m_image_mutex);
    static int color = 0;
    if (m_image_updated) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_background_texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_BGR, GL_UNSIGNED_BYTE, m_background_image.data);
        color = (color + 1) % 256;
        m_image_updated = false;
    }

    glUseProgram(m_background_shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_background_texture);
    glUniform1i(glGetUniformLocation(m_background_shader, "backgroundImage"), 0); 
    glBindVertexArray(m_quad_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

Renderer::Renderer(size_t width, size_t height, const std::string& shader_dir) : 
    m_width{width}, 
    m_height{height}, 
    m_shader_dir{shader_dir},
    m_should_close{false}
{
    // initialize and configure GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    std::cout << "[RENDERER]: Window created" << std::endl;

    // create window and make context current
    m_window = glfwCreateWindow(m_width, m_height, "Mixed Reality Demo", NULL, NULL);
    if (m_window == NULL)
    {
        // throw exception
        std::cerr << "[RENDERER]: GLFW Error" << std::endl;
    }
}

Renderer::~Renderer()
{
    glfwTerminate();
}

void Renderer::run()
{
    init_gl();
    init_ui();
    init_background_image();

    while (!m_should_close)
    {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        draw_background_image();

        // draw the UI on top of everything else
        draw_ui();

        glfwPollEvents();
        glfwSwapBuffers(m_window);
    }

    glfwSetWindowShouldClose(m_window, GL_TRUE);
}

void Renderer::set_background_image(cv::Mat &image)
{
    std::lock_guard<std::mutex> lock(m_image_mutex);

    if (!image.empty()) {
        m_background_image = image.clone();
        m_image_updated = true;
    }
}

void Renderer::set_close()
{
    m_should_close = true;
}