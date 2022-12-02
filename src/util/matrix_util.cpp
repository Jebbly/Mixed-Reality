#include "util/matrix_util.h"

cv::Mat ExpSO3(float x, float y, float z)
{
    cv::Mat I = cv::Mat::eye(3, 3, CV_32F);
    const float d2 = x * x + y * y + z * z;
    const float d = sqrt(d2);
    cv::Mat W = (cv::Mat_<float>(3, 3) << 
                  0, -z,  y,
                  z,  0, -x,
                 -y,  x,  0);
    if (d < EPSILON) {
        return (I + W + 0.5f * W * W);
    }
    
    return (I + W * sin(d) / d + W * W * (1.0f - cos(d)) / d2);
}

cv::Mat ExpSO3(const cv::Mat &v)
{
    return ExpSO3(v.at<float>(0), v.at<float>(1), v.at<float>(2));
}

glm::mat4 camera_projection(size_t width, size_t height, const std::string &camera_settings)
{
    cv::FileStorage settings(camera_settings, cv::FileStorage::READ);

    float fu = settings["Camera1.fx"];
    float fv = settings["Camera1.fy"];
    float u0 = settings["Camera1.cx"];
    float v0 = settings["Camera1.cy"];
    float near = 0.001;
    float far = 1000;

    const float L = -u0 * near / fu;
    const float R = (width - u0) * near / fu;
    const float T = -v0 * near / fv;
    const float B = (height - v0) * near / fv;

    glm::mat4 persp = glm::mat4(0.0f);
    persp[0][0] = 2.0f * near / (R - L);
    persp[1][1] = 2.0f * near / (T - B);
    
    persp[2][0] = (R + L) / (L - R);
    persp[2][1] = (T + B) / (B - T);
    persp[2][2] = (far + near) / (far - near);
    persp[2][3] = 1.0;
    
    persp[3][2] = (2.0f * far * near) / (near - far);

    return persp;
}

glm::mat4 glm_from_cv(const cv::Mat &cv_matrix)
{
    glm::mat4 glm_matrix = glm::mat4(1.0f);

    glm_matrix[0][0] = cv_matrix.at<float>(0, 0);
    glm_matrix[0][1] = cv_matrix.at<float>(1, 0);
    glm_matrix[0][2] = cv_matrix.at<float>(2, 0);
    glm_matrix[0][3] = 0.0f;

    glm_matrix[1][0] = cv_matrix.at<float>(0, 1);
    glm_matrix[1][1] = cv_matrix.at<float>(1, 1);
    glm_matrix[1][2] = cv_matrix.at<float>(2, 1);
    glm_matrix[1][3] = 0.0f;

    glm_matrix[2][0] = cv_matrix.at<float>(0, 2);
    glm_matrix[2][1] = cv_matrix.at<float>(1, 2);
    glm_matrix[2][2] = cv_matrix.at<float>(2, 2);
    glm_matrix[2][3] = 0.0f;

    glm_matrix[3][0] = cv_matrix.at<float>(0, 3);
    glm_matrix[3][1] = cv_matrix.at<float>(1, 3);
    glm_matrix[3][2] = cv_matrix.at<float>(2, 3);
    glm_matrix[3][3] = 1.0f;

    return glm_matrix;
}