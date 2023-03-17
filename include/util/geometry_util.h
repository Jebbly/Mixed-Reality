#ifndef GEOMETRY_UTIL_H
#define GEOMETRY_UTIL_H

#include <iostream>
#include <tuple>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <opencv2/core/core.hpp>
#include <MapPoint.h>
#include <stb_image.h>

#include "util/matrix_util.h"
#include "util/shader_util.h"

// Attributes needed to draw a screen quad for the background image
const float quad_vertices[] = {
    -1.0f, -1.0f, 
    -1.0f,  1.0f, 
    1.0f,  1.0f, 
    1.0f, -1.0f
};

const float quad_texcoords[] = {
    0.0f, 0.0f, 
    0.0f, 1.0f, 
    1.0f, 1.0f, 
    1.0f, 0.0f
};

const int quad_indices[] = {
    0, 2, 1,
    3, 2, 0
};

// A Plane defines the local coordinate space for each inserted object.
class Plane
{
private:
    cv::Mat m_origin, m_normal;
    float m_orientation;
    glm::mat4 m_model_matrix;

public:
    Plane(const cv::Mat &origin, const cv::Mat &normal, float orienation);
    Plane(const std::vector<ORB_SLAM3::MapPoint*> &plane_points, const cv::Mat &camera_pose);

    const glm::mat4& get_model_matrix() const;
    std::tuple<cv::Mat, cv::Mat, float> get_plane_information() const;

private:
    void recompute_model_matrix();
};

Plane* detect_plane(const std::vector<ORB_SLAM3::MapPoint*> &curr_map_points,
                    const std::vector<cv::KeyPoint> &curr_key_points,
                    const cv::Mat &curr_camera_pose);

struct Vertex 
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

enum class TextureType {
    TEXTURE_DIFFUSE,
    TEXTURE_SPECULAR,
};

struct Texture 
{
    unsigned int id;
    TextureType type;
    std::string filepath;
};

// A Mesh contains the actual geometry data.
class Mesh 
{
private:
    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;
    std::vector<Texture> m_textures;

    unsigned int m_vao, m_vbo, m_ebo;

public:
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    void draw(Shader &shader);

private:
    void setup_mesh();
};

// A Model consists of multiple Meshes.
class Model 
{
private:
    std::vector<Mesh> m_meshes;
    std::string m_directory;
    std::vector<Texture> m_loaded_textures;

public:
    Model() = default;
    Model(const std::string &filepath);
    void draw(Shader &shader);

private:
    // Helper loader functions
    void process_node(aiNode *node, const aiScene *scene);
    Mesh process_mesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> load_textures(aiMaterial *mat, aiTextureType type);
    unsigned int load_texture_file(const std::string &filepath, bool gamma = false);
};

struct Transformation {
    glm::vec3 translation;
    glm::vec3 scale;
    glm::vec3 rotation;
    float t;

    void update(float timestep);

    glm::mat4 transform_matrix();
};

// The Scene holds onto a single model object
// and instantiates it in multiple places.
// There isn't really enough geometry to warrant
// object instancing at the moment.
class Scene
{
private:
    std::string m_filepath;
    Model m_model;
    std::vector<Plane*> m_planes;
    std::vector<Transformation> m_transforms;

public:
    Scene(const std::string &filepath);
    void load();

    void draw(Shader &shader);
    void add_object(Plane *plane);
    void update(float timestep);
};

#endif // GEOMETRY_UTIL_H