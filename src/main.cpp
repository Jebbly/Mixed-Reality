#include <iostream>

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

    while(!glfwWindowShouldClose(window))
    {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}