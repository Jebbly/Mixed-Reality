#ifndef SHADER_UTIL_H
#define SHADER_UTIL_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>

struct Light
{
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
};

std::string read_shader(const std::string& shader_path);
int compile_shader(const std::string& shader_path, GLenum type);
int create_program(const std::string& vertex_path, const std::string& fragment_path);

#endif // SHADER_UTIL_H