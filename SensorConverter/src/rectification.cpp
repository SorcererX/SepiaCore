/*
Copyright (c) 2012-2015, Kai Hugo Hustoft Endresen <kai.endresen@gmail.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "rectification.h"

Rectification::Rectification( std::string intrinsic, std::string extrinsic )
{
    m_intrinsicFilename = intrinsic;
    m_extrinsicFilename = extrinsic;
}

bool Rectification::initialize( int width, int height )
{
    cv::Mat Q;
    cv::Size img_size = cv::Size( width, height );
    // reading intrinsic parameters
    cv::FileStorage fs(m_intrinsicFilename, cv::FileStorage::READ );
    if(!fs.isOpened())
    {
        printf("Failed to open file %s\n", m_intrinsicFilename.c_str() );
        return false;
    }

    cv::Mat M1, D1, M2, D2;
    fs["M1"] >> M1;
    fs["D1"] >> D1;
    fs["M2"] >> M2;
    fs["D2"] >> D2;

    fs.open(m_extrinsicFilename, cv::FileStorage::READ );
    if(!fs.isOpened())
    {
        printf("Failed to open file %s\n", m_extrinsicFilename.c_str() );
        return false;
    }

    cv::Mat R, T, R1, P1, R2, P2;
    fs["R"] >> R;
    fs["T"] >> T;

    cv::stereoRectify( M1, D1, M2, D2, img_size, R, T, R1, R2, P1, P2, Q, cv::CALIB_ZERO_DISPARITY, -1, img_size, &m_leftRoi, &m_rightRoi );

    cv::initUndistortRectifyMap(M1, D1, R1, P1, img_size, CV_16SC2, m_map11, m_map12);
    cv::initUndistortRectifyMap(M2, D2, R2, P2, img_size, CV_16SC2, m_map21, m_map22);
    return true;
}

void Rectification::remapLeft( cv::Mat* input, cv::Mat* output )
{
    cv::remap( *input, *output, m_map11, m_map12, cv::INTER_LINEAR);
}

void Rectification::remapRight( cv::Mat* input, cv::Mat* output )
{
    cv::remap( *input, *output, m_map21, m_map22, cv::INTER_LINEAR);
}
