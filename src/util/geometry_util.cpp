#include "util/geometry_util.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Plane::Plane(const std::vector<ORB_SLAM3::MapPoint*> &plane_points, const cv::Mat &camera_pose) : 
    map_points(plane_points)
{
    orientation = -3.14f / 2 + ((float) rand() / RAND_MAX) * 3.14f;
    recompute(camera_pose);
}

void Plane::recompute(const cv::Mat &camera_pose)
{
    const int N = map_points.size();

    // Recompute plane with all points
    cv::Mat A = cv::Mat(N, 4, CV_32F);
    A.col(3) = cv::Mat::ones(N, 1, CV_32F);

    origin = cv::Mat::zeros(3,1,CV_32F);

    int num_points = 0;
    for (int i = 0; i < N; i++)
    {
        ORB_SLAM3::MapPoint* map_point = map_points[i];
        if (!map_point->isBad())
        {
            cv::Mat world_pos = ORB_SLAM3::Converter::toCvMat(map_point->GetWorldPos());
            origin += world_pos;
            A.row(num_points).colRange(0,3) = world_pos.t();
            num_points++;
        }
    }
    A.resize(num_points);

    cv::Mat u, w, vt;
    cv::SVDecomp(A, w, u, vt, cv::SVD::MODIFY_A | cv::SVD::FULL_UV);

    float a = vt.at<float>(3, 0);
    float b = vt.at<float>(3, 1);
    float c = vt.at<float>(3, 2);
    std::cout << "[PLANE]: Plane coefficients: " << a << " " << b << " " << c << std::endl;

    origin = origin * (1.0f / num_points);
    const float f = 1.0f / sqrt(a * a + b * b + c * c);

    cv::Mat Oc = -camera_pose.colRange(0,3).rowRange(0,3).t() * camera_pose.rowRange(0,3).col(3);
    cv::Mat XC = Oc - origin;

    if ((XC.at<float>(0) * a + XC.at<float>(1) * b + XC.at<float>(2) * c) > 0)
    {
        a = -a;
        b = -b;
        c = -c;
    }

    const float nx = a * f;
    const float ny = b * f;
    const float nz = c * f;

    normal = (cv::Mat_<float>(3,1) << nx, ny, nz);
    cv::Mat up = (cv::Mat_<float>(3,1) << 0.0f, 1.0f, 0.0f);
    cv::Mat v = up.cross(normal);
    const float sa = cv::norm(v);
    const float ca = up.dot(normal);
    const float ang = atan2(sa, ca);

    cv::Mat transform = cv::Mat::eye(4, 4, CV_32F);
    transform.rowRange(0, 3).colRange(0, 3) = ExpSO3(v * ang / sa) * ExpSO3(up * orientation);
    origin.copyTo(transform.col(3).rowRange(0,3));
    model_matrix = glm_from_cv(transform);
}

Plane* detect_plane(const std::vector<ORB_SLAM3::MapPoint*> &curr_map_points, 
                    const std::vector<cv::KeyPoint> &curr_key_points, 
                    const cv::Mat &curr_camera_pose)
{
    // Retrieve 3D points
    std::vector<cv::Mat> points;
    std::vector<ORB_SLAM3::MapPoint*> map_points;
	std::vector<cv::KeyPoint> key_points;

    for (int i = 0; i < curr_map_points.size(); i++)
    {
        ORB_SLAM3::MapPoint* map_point = curr_map_points[i];
        if (map_point)
        {
            if (map_point->Observations() > 5)
            {
                points.push_back(ORB_SLAM3::Converter::toCvMat(map_point->GetWorldPos()));
                map_points.push_back(map_point);
				key_points.push_back(curr_key_points[i]);
            }
        }
    }

    const int N = points.size();

    if (N < 50)
        return nullptr;

    // Indices for minimum set selection
    std::vector<size_t> all_indices;
    std::vector<size_t> available_indices;

    for (int i = 0; i < N; i++)
    {
        all_indices.push_back(i);
    }

    float best_dist = 1e10;
    std::vector<float> best_dists;
	int best_it = 0;

	std::vector<int> plane_points;

    // RANSAC
    for (int n = 0; n < 50; n++)
    {
        available_indices = all_indices;

        cv::Mat A(3, 4, CV_32F);
        A.col(3) = cv::Mat::ones(3, 1, CV_32F);

		std::vector<int> plane_point_indices;
    
        // Get min set of points
        for (size_t i = 0; i < 3; i++)
        {
            int rand_idx = DUtils::Random::RandomInt(0, available_indices.size() - 1);
            int idx = available_indices[rand_idx];
			plane_point_indices.push_back(idx);

            A.row(i).colRange(0, 3) = points[idx].t();

            available_indices[rand_idx] = available_indices.back();
            available_indices.pop_back();
        }

        cv::Mat u, w, vt;
        cv::SVDecomp(A, w, u, vt, cv::SVD::MODIFY_A | cv::SVD::FULL_UV);

        const float a = vt.at<float>(3, 0);
        const float b = vt.at<float>(3, 1);
        const float c = vt.at<float>(3, 2);
        const float d = vt.at<float>(3, 3);

        std::vector<float> distances(N, 0);

        const float f = 1.0f / sqrt(a * a + b * b + c * c + d * d);
        for (int i = 0; i < N; i++)
        {
            distances[i] = fabs(points[i].at<float>(0) * a + points[i].at<float>(1) * b + points[i].at<float>(2) * c + d) * f;
        }

        std::vector<float> sorted = distances;
        std::sort(sorted.begin(), sorted.end());

        int nth = max((int) (0.2 * N), 20);
        const float median_dist = sorted[nth];

        if(median_dist < best_dist)
        {
			plane_points = plane_point_indices;
            best_dist = median_dist;
            best_dists = distances;
			best_it = n;
        }
	}

	std::cout << "[PLANE]: Best dist after RANSAC: " << best_dist << "\n";

    // Compute threshold inlier/outlier
    const float threshold = 1.4 * best_dist;
    std::vector<bool> inlier_flags(N, false);
    int inlier_count = 0;
    for (int i = 0; i < N; i++)
    {
        if(best_dists[i] < threshold)
        {
            inlier_count++;
            inlier_flags[i] = true;
        }
    }

    std::vector<ORB_SLAM3::MapPoint*> inlier_map_points(inlier_count, nullptr);
    int inlier_idx = 0;
    for (int i = 0; i < N; i++)
    {
        if(inlier_flags[i])
        {
            inlier_map_points[inlier_idx] = map_points[i];
            inlier_idx++;
        }
    }

    return new Plane(inlier_map_points, curr_camera_pose);
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) : 
    m_vertices{vertices},
    m_indices{indices},
    m_textures{textures}
{
    setup_mesh();   
}

void Mesh::setup_mesh()
{
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    // Pass the vertex data and describe its layout
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, uv));

    // Also pass the index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void Mesh::draw(Shader &shader)
{
    unsigned int diffuse = 1, specular = 1, normal = 1;

    for (int i = 0; i < m_textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        Texture texture = m_textures[i];

        std::string name = "material.";
        switch (texture.type) {
            case TextureType::TEXTURE_DIFFUSE: {
                name += "texture_diffuse";
            }
            case TextureType::TEXTURE_SPECULAR: {
                name += "texture_specular";
            }
        }

        shader.set_int(name, i);
        glBindTexture(GL_TEXTURE_2D, texture.id);
    }

    // Draw the geometry after all the textures have been set
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(m_indices.size()), GL_UNSIGNED_INT, 0);
    
    // Reset things to the default
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}

Model::Model(const std::string &filepath)
{
    // Load model through file path
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs);	
	
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        std::cout << "[SCENE]: Assimp import error - " << import.GetErrorString() << std::endl;
        return;
    }
    m_directory = filepath.substr(0, filepath.find_last_of('/'));

    // stbi_set_flip_vertically_on_load(true);
    process_node(scene->mRootNode, scene);
} 

void Model::draw(Shader &shader)
{
    for (int i = 0; i < m_meshes.size(); i++) {
        m_meshes[i].draw(shader);
    }
}

// Helper loader functions
void Model::process_node(aiNode *node, const aiScene *scene)
{
    // Process node meshes
    for (int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
        m_meshes.push_back(process_mesh(mesh, scene));			
    }
    
    // Then process node children
    for (int i = 0; i < node->mNumChildren; i++)
    {
        process_node(node->mChildren[i], scene);
    }
}

Mesh Model::process_mesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for (int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;

        // Positions
        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;

        // Normals
        if (mesh->HasNormals()) {
            vertex.normal.x = mesh->mNormals[i].x;
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;
        }

        // Texture coordinates
        if (mesh->mTextureCoords[0]) {
            vertex.uv.x = mesh->mTextureCoords[0][i].x; 
            vertex.uv.y = mesh->mTextureCoords[0][i].y;
        }
        else {
            vertex.uv = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }
    
    // Process indices for each face
    for (int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);   
        }     
    }
    
    // Process materials for the mesh
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];   

    // Diffuse
    std::vector<Texture> diffuseMaps = load_textures(material, aiTextureType_DIFFUSE);
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    // Specular
    std::vector<Texture> specularMaps = load_textures(material, aiTextureType_SPECULAR);
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::load_textures(aiMaterial *mat, aiTextureType type)
{
    std::vector<Texture> textures;
    for (int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        std::string texture_filepath{str.C_Str()};

        // Check if texture was loaded already
        bool skip = false;
        for (int j = 0; j < m_loaded_textures.size(); j++)
        {
            if (m_loaded_textures[j].filepath == texture_filepath)
            {
                textures.push_back(m_loaded_textures[j]);
                skip = true;
                break;
            }
        }

        // Load the texture if it hasn't been loaded already
        if (!skip) {
            Texture texture;
            texture.id = load_texture_file(m_directory + '/' + texture_filepath);

            switch (type) {
                case aiTextureType_DIFFUSE: {
                    texture.type = TextureType::TEXTURE_DIFFUSE;
                    break;
                }
                case aiTextureType_SPECULAR: {
                    texture.type = TextureType::TEXTURE_SPECULAR;
                    break;
                }
                default: {
                    throw std::runtime_error("Texture type not supported!");
                }
            }

            texture.filepath = texture_filepath;
            textures.push_back(texture);
            m_loaded_textures.push_back(texture); 
        }
    }

    return textures;
}

unsigned int Model::load_texture_file(const std::string &filepath, bool gamma)
{
    unsigned int texture;
    glGenTextures(1, &texture);

    int width, height, channels;
    unsigned char *data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
    if (data)
    {
        GLenum format;

        switch (channels) {
            case 1: {
                format = GL_RED;
                break;
            }
            case 3: {
                format = GL_RGB;
                break;
            }
            case 4: {
                format = GL_RGBA;
                break;
            }
            default: {
                throw std::runtime_error("[SCENE]: Unsupported texture format");
            }
        }

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        throw std::runtime_error("[SCENE]: Texture failed to load from: " + filepath);
        stbi_image_free(data);
    }

    return texture;
}

void Transformation::update(float timestep)
{
    t += timestep;
}

glm::mat4 Transformation::transform_matrix()
{
    glm::mat4 transform = glm::mat4(1.0f);

    transform = glm::scale(transform, scale);

    transform = glm::rotate(transform, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    transform = glm::rotate(transform, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    // transform = glm::rotate(transform, glm::radians(150 * t), glm::vec3(0.0f, 0.0f, 1.0f));

    transform = glm::translate(transform, translation);
    // transform = glm::translate(transform, glm::vec3(glm::cos(t), glm::sin(t), 0));

    return transform;
}

Scene::Scene(const std::string &filepath) :
    m_filepath{filepath}
{
    // Start with an empty scene
}

void Scene::load()
{
    m_model = Model(m_filepath);
}

void Scene::draw(Shader &shader)
{
    for (int i = 0; i < m_planes.size(); i++) {
        shader.set_mat4("plane", m_planes[i]->model_matrix);
        shader.set_mat4("local", m_transforms[i].transform_matrix());
        m_model.draw(shader);
    }
}

void Scene::add_object(Plane *plane)
{
    m_planes.push_back(plane);
    Transformation transform;
    transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
    transform.translation = glm::vec3(0.0f, 0.0f, 0.0f);
    transform.t = 0.0f;
    m_transforms.push_back(transform);
}

void Scene::update(float timestep)
{
    for (int i = 0; i < m_planes.size(); i++) {
        // Update the local transformations here
        m_transforms[i].update(timestep);
    }
}