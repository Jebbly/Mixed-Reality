#ifndef LIGHT_H
#define LIGHT_H

#include <iostream>
#include <cstdlib>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <opencv2/core/core.hpp>

struct Light
{
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
};

class RandLightEstimator
{
private:
    int m_num_lights;
    std::vector<Light> m_lights;

public:
    RandLightEstimator(int num_lights);

    void estimate_lights(const cv::Mat &rgb_image, const cv::Mat &depth_image);
    const std::vector<Light>& get_lights() const;

private:
    // Helper function to normalize random number generation
    float rand_float() const;
};


#endif // LIGHT_H