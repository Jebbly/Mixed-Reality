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

    // Set of actions
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

    std::vector<GLubyte> empty(3 * m_width * m_height, 0);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, empty.data());

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

    // We also need a shader to only draw the background image
    m_background_shader = create_program(m_shader_dir + "/background_vertex.glsl", m_shader_dir + "/background_fragment.glsl");

    // Set the flag to indicate that there's no new data for now
    m_image_updated = false;

    std::cout << "[RENDERER]: Background image shaders initialized" << std::endl;
}

void Renderer::draw_background_image() 
{
    // need a mutex here
    // Get the most recently updated image
    // if the image has changed
    if (m_image_updated) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_background_texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_BGR, GL_UNSIGNED_BYTE, m_background_image.data);
        m_image_updated = false;
    }

    glUseProgram(m_background_shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_background_texture);
    glUniform1i(glGetUniformLocation(m_background_shader, "backgroundImage"), 0); 
    glBindVertexArray(m_quad_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void Renderer::init_objects()
{
    // Need a perspective matrix when drawing
    float fu = 726.28741455078, fv = 726.28741455078, u0 = 354.6496887207, v0 = 186.46566772461, zNear = 0.001, zFar = 1000;
    const float L = -(u0) * zNear / fu;
    const float R = +(m_width-u0) * zNear / fu;
    const float T = -(v0) * zNear / fv;
    const float B = +(m_height-v0) * zNear / fv;

    m_persp = glm::mat4(0.0f);
    m_persp[0][0] = 2 * zNear / (R-L);
    m_persp[1][1] = 2 * zNear / (T-B);
    
    m_persp[2][0] = (R+L)/(L-R);
    m_persp[2][1] = (T+B)/(B-T);
    m_persp[2][2] = (zFar +zNear) / (zFar - zNear);
    m_persp[2][3] = 1.0;
    
    m_persp[3][2] = (2*zFar*zNear)/(zNear - zFar);

    // The same cube is used for each object
    glGenVertexArrays(1, &m_object_vao);
    glBindVertexArray(m_object_vao);

    GLuint vertex_buffer = 0;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    GLuint index_buffer = 0;
    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

    // We also need a different shader for the actual objects
    m_object_shader = create_program(m_shader_dir + "/object_vertex.glsl", m_shader_dir + "/object_fragment.glsl");

    std::cout << "[RENDERER]: Object shaders initialized" << std::endl;
}

void Renderer::draw_object(size_t index)
{
    // Get the plane
    Plane* plane = m_planes[index];
    glUniformMatrix4fv(glGetUniformLocation(m_object_shader, "model"), 1, GL_FALSE, &plane->model_matrix[0][0]);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
}

void Renderer::draw_objects()
{
    if (m_camera_pose.empty()) {
        return;
    }

    // First bind the view and perspective models;
    // only the model matrix changes between objects
    glUseProgram(m_object_shader);
    glBindVertexArray(m_object_vao);

    glUniformMatrix4fv(glGetUniformLocation(m_object_shader, "persp"), 1, GL_FALSE, &m_persp[0][0]);

    glm::mat4 view = glm::mat4(1.0f);
    view[0][0] = m_camera_pose.at<float>(0,0);
    view[0][1] = m_camera_pose.at<float>(1,0);
    view[0][2] = m_camera_pose.at<float>(2,0);
    view[0][3] = 0.0;

    view[1][0] = m_camera_pose.at<float>(0,1);
    view[1][1] = m_camera_pose.at<float>(1,1);
    view[1][2] = m_camera_pose.at<float>(2,1);
    view[1][3] = 0.0;

    view[2][0] = m_camera_pose.at<float>(0,2);
    view[2][1] = m_camera_pose.at<float>(1,2);
    view[2][2] = m_camera_pose.at<float>(2,2);
    view[2][3] = 0.0;

    view[3][0] = m_camera_pose.at<float>(0,3);
    view[3][1] = m_camera_pose.at<float>(1,3);
    view[3][2] = m_camera_pose.at<float>(2,3);
    view[3][3] = 1.0;

    glUniformMatrix4fv(glGetUniformLocation(m_object_shader, "view"), 1, GL_FALSE, &view[0][0]);

    for (size_t i = 0; i < m_planes.size(); i++) {
        draw_object(i);
    }
}

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

Renderer::Renderer(size_t width, size_t height, const std::string& shader_dir) : 
    m_width{width}, 
    m_height{height}, 
    m_shader_dir{shader_dir},
    m_draw_key_points{false},
    m_add_object{false},
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
    for (int i = 0; i < m_planes.size(); i++) {
        delete m_planes[i];
    }
    
    glfwTerminate();
}

void Renderer::run()
{
    init_gl();
    init_ui();
    init_background_image();
    init_objects();

    std::unique_lock<std::mutex> lock(m_info_mutex, std::defer_lock);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    while (!m_should_close)
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        lock.lock();

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

        lock.unlock();

        // draw the UI on top of everything else
        draw_ui();

        glfwPollEvents();
        glfwSwapBuffers(m_window);
    }

    glfwSetWindowShouldClose(m_window, GL_TRUE);
}

void Renderer::set_info(const cv::Mat &rgb_image, const cv::Mat &depth_image, const cv::Mat &pose, 
                        const std::vector<ORB_SLAM3::MapPoint*> &map_points,
                        const std::vector<cv::KeyPoint> &key_points)
{
    std::lock_guard<std::mutex> lock(m_info_mutex);

    m_background_image = rgb_image.clone();
    m_completed_depth = depth_image.clone();
    m_camera_pose = pose.clone();

    m_map_points = map_points;
    m_key_points = key_points;

    m_image_updated = true;
}

void Renderer::set_close()
{
    m_should_close = true;
}