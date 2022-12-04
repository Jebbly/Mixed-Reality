#include "light_estimation.h"

// Interface definitions
LightEstimator::LightEstimator(int num_lights) :
    m_num_lights{num_lights}
{

}

LightEstimator::~LightEstimator()
{
    
}

const std::vector<Light>& LightEstimator::get_lights() const
{
    return m_lights;
}

// Implementation definitions
RandLightEstimator::RandLightEstimator(int num_lights) :
    LightEstimator{num_lights}
{

}

void RandLightEstimator::estimate_lights(const cv::Mat &rgb_image, const cv::Mat &depth_image)
{
    // RGB and depth images may be used in certain light estimation models,
    // but this light estimator just approximates the color and randomizes positions.
    for (int i = 0; i < m_num_lights; i++) {
        Light light;
        light.position = glm::vec3(rand_float(), rand_float(), rand_float());
        light.color = glm::vec3(1.0f, 1.0f, 0.75f);
        light.intensity = abs(rand_float()) * 5.0f;
        m_lights.push_back(light);
    }

    std::cout << "[LIGHT ESTIMATOR]: Light source estimation complete" << std::endl;
}

float RandLightEstimator::rand_float() const
{
    return ((float) std::rand() / RAND_MAX) * 2.0f - 1.0f;
}