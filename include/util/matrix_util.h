#ifndef MATRIX_UTIL_H
#define MATRIX_UTIL_H

#include <glm/matrix.hpp>
#include <opencv2/core/core.hpp>

const float EPSILON = 1e-4;
cv::Mat ExpSO3(float x, float y, float z);
cv::Mat ExpSO3(const cv::Mat &v);

glm::mat4 camera_projection(size_t width, size_t height, const std::string &camera_settings);
glm::mat4 glm_from_cv(const cv::Mat &cv_matrix);

#endif // MATRIX_UTIL_H