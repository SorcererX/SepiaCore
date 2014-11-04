#ifndef RECTIFICATION_H
#define RECTIFICATION_H
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/contrib/contrib.hpp"
#include <string>

class Rectification
{
public:
    Rectification( std::string intrinsic_filename, std::string extrinsic_filename );
    bool initialize( int width, int height );
    void remapLeft( cv::Mat* input, cv::Mat* output );
    void remapRight( cv::Mat* input, cv::Mat* output );

private:
    std::string m_intrinsicFilename;
    std::string m_extrinsicFilename;
    cv::Mat m_map11;
    cv::Mat m_map12;
    cv::Mat m_map21;
    cv::Mat m_map22;
    cv::Rect m_leftRoi;
    cv::Rect m_rightRoi;
};

#endif // RECTIFICATION_H
