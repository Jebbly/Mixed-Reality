#include "renderer.h"

Renderer::Renderer(size_t width, size_t height, const std::string &settings, const std::string& shaders) : 
    m_width{width}, 
    m_height{height}, 
    m_camera_settings{settings},
    m_shader_dir{shaders},
    m_image_updated{false},
    m_draw_key_points{false},
    m_add_object{false},
    m_should_close{false}
{
    // Most initialization happens when run() is called on a separate thread
}

Renderer::~Renderer()
{
    for (int i = 0; i < m_planes.size(); i++) {
        delete m_planes[i];
    }
    
    glfwTerminate();
}

void Renderer::run()
{
    init_window();
    init_gl();
    init_framebuffer();
    init_shaders();
    init_images();
    init_objects();
    init_ui();

    std::unique_lock<std::mutex> slam_lock(m_slam_mutex, std::defer_lock);
    std::unique_lock<std::mutex> image_lock(m_image_mutex, std::defer_lock);
    std::unique_lock<std::mutex> light_lock(m_light_mutex, std::defer_lock);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST); 
    while (!m_should_close)
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        slam_lock.lock();
        image_lock.lock();
        light_lock.lock();

        if (m_draw_key_points) {
            draw_key_points();
        }
        
        draw_background_image();

        if (m_add_object) {
            Plane* plane = add_object(m_map_points, m_key_points, m_camera_pose);
            if (plane) {
                std::cout << "[RENDERER]: New object added" << std::endl;
                m_planes.push_back(plane);
            } else {
                std::cout << "[RENDERER]: No plane detected to add object" << std::endl;
            }
            m_add_object = false;
        }

        glClear(GL_DEPTH_BUFFER_BIT);
        draw_objects();

        m_image_updated = false;

        slam_lock.unlock();
        image_lock.unlock();
        light_lock.unlock();

        // draw the UI on top of everything else
        draw_ui();

        glfwPollEvents();
        glfwSwapBuffers(m_window);
    }

    glfwSetWindowShouldClose(m_window, GL_TRUE);
}

void Renderer::close()
{
    m_should_close = true;
}

void Renderer::set_slam(const cv::Mat &pose, 
                        const std::vector<ORB_SLAM3::MapPoint*> &map_points,
                        const std::vector<cv::KeyPoint> &key_points)
{
    std::lock_guard<std::mutex> lock(m_slam_mutex);

    m_camera_pose = pose.clone();
    m_map_points = map_points;
    m_key_points = key_points;
}

void Renderer::set_images(const cv::Mat &rgb_image, const cv::Mat &depth_image)
{
    std::lock_guard<std::mutex> lock(m_image_mutex);

    m_background_image = rgb_image.clone();
    m_completed_depth = depth_image.clone();
    m_image_updated = true;
}

void Renderer::set_lights(const std::vector<Light> &lights)
{
    std::lock_guard<std::mutex> lock(m_light_mutex);

    m_lights = lights;
}

static void glfw_resize_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Initialization helpers
void Renderer::init_window()
{
    // Initialize and configure GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Disable window resizing for now
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    // Create window and make context current
    m_window = glfwCreateWindow(m_width, m_height, "Mixed Reality Demo", NULL, NULL);
    if (m_window == NULL)
    {
        std::cerr << "[RENDERER]: GLFW Error" << std::endl;
    }

    glfwSetWindowAspectRatio(m_window, m_width, m_height);
    glfwSetWindowSizeCallback(m_window, glfw_resize_callback);
    glfwSetWindowSizeLimits(m_window, m_width, m_height, GLFW_DONT_CARE, GLFW_DONT_CARE);

    std::cout << "[RENDERER]: Window created" << std::endl;
}

void Renderer::init_gl()
{
    glfwMakeContextCurrent(m_window);

    // Load address of OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
          std::cerr << "[RENDERER]: GLAD Error" << std::endl;
    }

    glViewport(0, 0, m_width, m_height);

    m_persp = camera_projection(m_width, m_height, m_camera_settings);

    std::cout << "[RENDERER]: OpenGL initialized" << std::endl;
}

void Renderer::init_framebuffer()
{
    glGenFramebuffers(1, &m_geometry_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_geometry_fbo);

    glGenTextures(1, &m_positions);
    glBindTexture(GL_TEXTURE_2D, m_positions);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_positions, 0);

    glGenTextures(1, &m_normals);
    glBindTexture(GL_TEXTURE_2D, m_normals);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_normals, 0);

    unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);

    unsigned int rbo_depth;
    glGenRenderbuffers(1, &rbo_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth);

    std::cout << "[RENDERER]: Geometry framebuffer created" << std::endl;
}

void Renderer::init_shaders()
{
    // Three separate shader programs are used:
    // 1 for drawing the RGB image in the background,
    // and 2 for the deferred rendering pipeline.
    m_image_shader = create_program(m_shader_dir + "/background_vert.glsl", 
                                    m_shader_dir + "/background_frag.glsl");
    
    m_geometry_shader = create_program(m_shader_dir + "/geometry_vert.glsl", 
                                       m_shader_dir + "/geometry_frag.glsl");

    m_deferred_shader = create_program(m_shader_dir + "/deferred_vert.glsl", 
                                       m_shader_dir + "/deferred_frag.glsl");

    std::cout << "[RENDERER]: Shaders compiled and linked" << std::endl;
}

void Renderer::init_images()
{
    // Configure textures in OpenGL to store the background and depth
    glGenTextures(1, &m_background_texture);
    glBindTexture(GL_TEXTURE_2D, m_background_texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    std::vector<GLubyte> empty_rgb(3 * m_width * m_height, 255);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, empty_rgb.data());

    glGenTextures(1, &m_depth_texture);
    glBindTexture(GL_TEXTURE_2D, m_depth_texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, m_width, m_height, 0, GL_RED, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    std::vector<GLfloat> empty_depth(m_width * m_height, 1.0f);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RED, GL_FLOAT, empty_depth.data());

    // Draw a screen quad to sample the background image
    glGenVertexArrays(1, &m_quad_vao);
    glBindVertexArray(m_quad_vao);

    GLuint vertex_buffer = 0;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    GLuint texcoord_buffer = 0;
    glGenBuffers(1, &texcoord_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, texcoord_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_texcoords), quad_texcoords, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(1);

    GLuint index_buffer = 0;
    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices), quad_indices, GL_STATIC_DRAW);

    std::cout << "[RENDERER]: Quad VAO created" << std::endl;
}

void Renderer::init_objects()
{
    // The same cube is used for each object
    glGenVertexArrays(1, &m_geometry_vao);
    glBindVertexArray(m_geometry_vao);

    GLuint vertex_buffer = 0;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    GLuint normal_buffer = 0;
    glGenBuffers(1, &normal_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_normals), cube_normals, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(1);

    GLuint index_buffer = 0;
    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

    std::cout << "[RENDERER]: Geometry VAO created" << std::endl;
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

// Renderer drawing helpers
void Renderer::draw_key_points()
{
    for (int i = 0; i < m_key_points.size(); i++)
    {
        if (m_map_points[i])
        {
            cv::circle(m_background_image, m_key_points[i].pt, 2, cv::Scalar(0, 255, 0), -1);
        }
    }
}

void Renderer::draw_background_image() 
{
    // Render this to the window's resolution
    int width, height;
    glfwGetWindowSize(m_window, &width, &height);
    glViewport(0, 0, width, height);

    // Get the most recently updated image if it changed
    if (m_image_updated) {
        glBindTexture(GL_TEXTURE_2D, m_background_texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_BGR, GL_UNSIGNED_BYTE, m_background_image.data);
    }

    // The background image is drawn directly to the screen framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(m_image_shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_background_texture);
    glUniform1i(glGetUniformLocation(m_image_shader, "backgroundImage"), 0); 
    glBindVertexArray(m_quad_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void Renderer::draw_objects()
{
    if (m_camera_pose.empty()) {
        return;
    }

    // We first render everything at the camera's resolution
    glViewport(0, 0, m_width, m_height);

    // First draw to a geometry buffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_geometry_fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // First bind the view and perspective matrices;
    // only the model matrix changes between objects
    glUseProgram(m_geometry_shader);
    glBindVertexArray(m_geometry_vao);

    glUniformMatrix4fv(glGetUniformLocation(m_geometry_shader, "persp"), 1, GL_FALSE, &m_persp[0][0]);

    glm::mat4 view = glm_from_cv(m_camera_pose);
    glUniformMatrix4fv(glGetUniformLocation(m_geometry_shader, "view"), 1, GL_FALSE, &view[0][0]);

    for (size_t i = 0; i < m_planes.size(); i++) {
        Plane* plane = m_planes[i];
        glUniformMatrix4fv(glGetUniformLocation(m_geometry_shader, "model"), 1, GL_FALSE, &plane->model_matrix[0][0]);
        glDrawElements(GL_TRIANGLES, sizeof(cube_indices) / sizeof(int), GL_UNSIGNED_INT, nullptr);
    }

    // Then we use the data in the deferred shading pass,
    // which should be at the window's resolution
    int width, height;
    glfwGetWindowSize(m_window, &width, &height);
    glViewport(0, 0, width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_positions);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_normals);
    if (m_image_updated) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_depth_texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RED, GL_FLOAT, m_completed_depth.data);
    }
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_depth_texture);

    glUseProgram(m_deferred_shader);
    glUniform1i(glGetUniformLocation(m_deferred_shader, "gPosition"), 0);
    glUniform1i(glGetUniformLocation(m_deferred_shader, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(m_deferred_shader, "depthTexture"), 2);

    for (int i = 0; i < m_lights.size(); i++) {
        std::string position = "lights[" + std::to_string(i) + "].position";
        glUniform3fv(glGetUniformLocation(m_deferred_shader, position.c_str()), 1, &m_lights[i].position[0]);
        
        std::string color = "lights[" + std::to_string(i) + "].color";
        glUniform3fv(glGetUniformLocation(m_deferred_shader, color.c_str()), 1, &m_lights[i].color[0]);
        
        std::string intensity = "lights[" + std::to_string(i) + "].intensity";
        glUniform1f(glGetUniformLocation(m_deferred_shader, intensity.c_str()), m_lights[i].intensity);
    }

    glBindVertexArray(m_quad_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void Renderer::draw_ui()
{
    static int counter = 0;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Actions");   

    // Display the possible set of actions
    ImGui::Checkbox("Draw Keypoints", &m_draw_key_points);

    if (ImGui::Button("Insert Object")) {
        m_add_object = true;
    }

    // Also display application statistics
    ImGui::NewLine();
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}