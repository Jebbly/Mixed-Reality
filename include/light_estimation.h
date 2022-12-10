#ifndef LIGHT_ESTIMATION_H
#define LIGHT_ESTIMATION_H

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

// Base class defines an interface for light source estimation.
class LightEstimator 
{
protected:
    int m_num_lights;
    std::vector<Light> m_lights;

public:
    LightEstimator(int num_lights);
    virtual ~LightEstimator();

    virtual void estimate_lights(const cv::Mat &rgb_image, const cv::Mat &depth_image) = 0;
    const std::vector<Light>& get_lights() const;
};

// This implementation randomly generates positions to approximate light sources.
class RandLightEstimator : public LightEstimator
{
private:
    bool m_estimated;
    
public:
    RandLightEstimator(int num_lights);

    virtual void estimate_lights(const cv::Mat &rgb_image, const cv::Mat &depth_image);

private:
    // Helper function to normalize random number generation
    float rand_float() const;
};


#endif // LIGHT_ESTIMATION_H