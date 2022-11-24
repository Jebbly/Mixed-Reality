#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <glad/glad.h>

std::string read_shader(const std::string& shader_path);
int compile_shader(const std::string& shader_path, GLenum type);
int create_program(const std::string& vertex_path, const std::string& fragment_path);