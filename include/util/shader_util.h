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

// Shader creation helpers
std::string read_shader(const std::string& shader_path);
int compile_shader(const std::string& shader_path, GLenum type);
int create_program(const std::string& vertex_path, const std::string& fragment_path);

class Shader
{
private:
    unsigned int m_id;

public:
    Shader() = default;
    Shader(const std::string &vertex_path, const std::string &fragment_path);
    void use();

    // Uniform setters
    void set_int(const std::string &name, int value) const;
    void set_float(const std::string &name, float value) const;
    void set_vec3(const std::string &name, const glm::vec3 &value) const;
    void set_mat4(const std::string &name, const glm::mat4 &value) const;
};

#endif // SHADER_UTIL_H