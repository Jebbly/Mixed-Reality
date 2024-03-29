#include "util/shader_util.h"

std::string read_shader(const std::string& shader_path) {
	std::fstream file_stream;
	std::stringstream string_stream;
	std::string code;
	try {
		file_stream.open(shader_path);
		string_stream << file_stream.rdbuf();
		file_stream.close();
		code = string_stream.str();
	}
	catch (std::ifstream::failure e) {
		std::cerr << "Failed to read shader" << std::endl;
	}

	return code;
}

int compile_shader(const std::string& shader_path, GLenum type) {
    int shader = glCreateShader(type);
    std::string shader_code = read_shader(shader_path);
    const char* code = shader_code.c_str();
    glShaderSource(shader, 1, &code, NULL);
    glCompileShader(shader);

    char log[512];
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, log);
        std::cerr << "Shader compilation failed for " << shader_path << ": \n" << log << std::endl;
    };

    return shader;
}

int create_program(const std::string& vertex_path, const std::string& fragment_path) {
    int vertex_shader = compile_shader(vertex_path, GL_VERTEX_SHADER);
    int fragment_shader = compile_shader(fragment_path, GL_FRAGMENT_SHADER);

    int program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    char log[512];
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, log);
        std::cerr << "Shader program linking failed: \n" << log << std::endl;
    }

    return program;
}

Shader::Shader(const std::string &vertex_path, const std::string &fragment_path)
{
    m_id = create_program(vertex_path, fragment_path);
}

void Shader::use()
{
    glUseProgram(m_id);
}

// Uniform setters
void Shader::set_int(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(m_id, name.c_str()), value); 
}

void Shader::set_float(const std::string &name, float value) const
{
    glUniform1f(glGetUniformLocation(m_id, name.c_str()), value); 
}


void Shader::set_vec3(const std::string &name, const glm::vec3 &value) const
{
    glUniform3fv(glGetUniformLocation(m_id, name.c_str()), 1, &value[0]);
}

void Shader::set_mat4(const std::string &name, const glm::mat4 &value) const
{
    glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, &value[0][0]);
}