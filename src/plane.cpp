#include "plane.h"

const float eps = 1e-4;
cv::Mat ExpSO3(const float &x, const float &y, const float &z)
{
    cv::Mat I = cv::Mat::eye(3,3,CV_32F);
    const float d2 = x*x+y*y+z*z;
    const float d = sqrt(d2);
    cv::Mat W = (cv::Mat_<float>(3,3) << 0, -z, y,
                 z, 0, -x,
                 -y,  x, 0);
    if(d<eps)
        return (I + W + 0.5f*W*W);
    else
        return (I + W*sin(d)/d + W*W*(1.0f-cos(d))/d2);
}

cv::Mat ExpSO3(const cv::Mat &v)
{
    return ExpSO3(v.at<float>(0),v.at<float>(1),v.at<float>(2));
}

Plane::Plane(const std::vector<ORB_SLAM3::MapPoint*> &vMPs, const cv::Mat &Tcw):mvMPs(vMPs),mTcw(Tcw.clone())
{
	// std::cout << "[DEBUG] Recomputing plane !!!\n";
    rang = 0; // -3.14f/2+((float)rand()/RAND_MAX)*3.14f;
    Recompute();
}

void Plane::Recompute()
{
    const int N = mvMPs.size();

    // Recompute plane with all points
    cv::Mat A = cv::Mat(N,4,CV_32F);
    A.col(3) = cv::Mat::ones(N,1,CV_32F);

    o = cv::Mat::zeros(3,1,CV_32F);
	// Eigen::Vector3f o = Eigen::Vector3f::Zero();

    int nPoints = 0;
    for(int i=0; i<N; i++)
    {
        ORB_SLAM3::MapPoint* pMP = mvMPs[i];
        if(!pMP->isBad())
        {
            cv::Mat Xw = ORB_SLAM3::Converter::toCvMat(pMP->GetWorldPos());
            // Eigen::Vector3f Xw = pMP->GetWorldPos();

            // std::cout << "POINT" << std::endl;
            // std::cout << Xw << std::endl;
            o+=Xw;

            A.row(nPoints).colRange(0,3) = Xw.t();
			// A.at<float>(nPoints, 0) = Xw(0);
			// A.at<float>(nPoints, 1) = Xw(1);
			// A.at<float>(nPoints, 2) = Xw(2);

            nPoints++;
        }
    }
    A.resize(nPoints);

    cv::Mat u,w,vt;
    cv::SVDecomp(A,w,u,vt,cv::SVD::MODIFY_A | cv::SVD::FULL_UV);

    float a = vt.at<float>(3,0);
    float b = vt.at<float>(3,1);
    float c = vt.at<float>(3,2);

    std::cout << "Plane coefficients: " << a << " " << b << " " << c << std::endl;

    o = o*(1.0f/nPoints);
    const float f = 1.0f/sqrt(a*a+b*b+c*c);

    // Compute XC just the first time
    if(XC.empty())
    {
        cv::Mat Oc = -mTcw.colRange(0,3).rowRange(0,3).t()*mTcw.rowRange(0,3).col(3);
        XC = Oc-o;
		// XC.at<float>(0) = Oc.at<float>(0) - o(0);
		// XC.at<float>(1) = Oc.at<float>(1) - o(1);
		// XC.at<float>(2) = Oc.at<float>(2) - o(2);
    }

    if((XC.at<float>(0)*a+XC.at<float>(1)*b+XC.at<float>(2)*c)>0)
    {
        a=-a;
        b=-b;
        c=-c;
    }

    const float nx = a*f;
    const float ny = b*f;
    const float nz = c*f;

    n = (cv::Mat_<float>(3,1)<<nx,ny,nz);

    cv::Mat up = (cv::Mat_<float>(3,1) << 0.0f, 1.0f, 0.0f);

    cv::Mat v = up.cross(n);
    const float sa = cv::norm(v);
    const float ca = up.dot(n);
    const float ang = atan2(sa,ca);
    Tpw = cv::Mat::eye(4,4,CV_32F);

    std::cout << "PLANE" << std::endl;
    std::cout << Tpw << std::endl;


    Tpw.rowRange(0,3).colRange(0,3) = ExpSO3(v*ang/sa)*ExpSO3(up*rang);
    std::cout << "PLANE" << std::endl;
    std::cout << Tpw << std::endl;
	// colRange rowRange only cover left not right
    o.copyTo(Tpw.col(3).rowRange(0,3));

    std::cout << "PLANE" << std::endl;
    std::cout << Tpw << std::endl;

    glTpw[0][0] = Tpw.at<float>(0,0);
    glTpw[0][1] = Tpw.at<float>(1,0);
    glTpw[0][2] = Tpw.at<float>(2,0);
    glTpw[0][3] = 0.0;

    glTpw[1][0] = Tpw.at<float>(0,1);
    glTpw[1][1] = Tpw.at<float>(1,1);
    glTpw[1][2] = Tpw.at<float>(2,1);
    glTpw[1][3] = 0.0;

    glTpw[2][0] = Tpw.at<float>(0,2);
    glTpw[2][1] = Tpw.at<float>(1,2);
    glTpw[2][2] = Tpw.at<float>(2,2);
    glTpw[2][3] = 0.0;

    glTpw[3][0] = Tpw.at<float>(0,3);
    glTpw[3][1] = Tpw.at<float>(1,3);
    glTpw[3][2] = Tpw.at<float>(2,3);
    glTpw[3][3] = 1.0;
}