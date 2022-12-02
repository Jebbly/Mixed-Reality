#include "plane.h"

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

Plane* add_object(const std::vector<ORB_SLAM3::MapPoint*> &curr_map_points, 
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