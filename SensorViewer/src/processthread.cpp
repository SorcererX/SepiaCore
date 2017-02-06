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

#include "processthread.h"
#include <chrono>
#include <ratio>
#include <boost/thread.hpp>
#include <opencv2/opencv.hpp>
#include <settings.h>
#include <sepia/writer.h>
#include <sepia/reader.h>

namespace
{
void adjustWb( cv::Mat* input, double Kr, double Kg, double Kb )
{
    unsigned char* image = (unsigned char*) input->data;
    int p = 0;
    for( int y = 0; y < input->rows; y++ )
    {
        if( ( y & 1 ) == 0)
        {
            for( int x = 0; x < input->cols; x++ )
            {
                if( ( x & 1 ) == 0 )
                {
                    p = image[y * input->cols + x ] * Kr;
                }
                else
                {
                    p = image[y * input->cols + x ] * Kg;
                }
                image[y * input->cols + x ]= std::min( p, 255 );
            }
        }
        else
        {
            for( int x = 0; x < input->cols; x++ )
            {
                if( ( x & 1 )  == 1 )
                {
                    p = image[y * input->cols + x ] * Kb;
                }
                else
                {
                    p = image[y * input->cols + x ] * Kg;
                }
                image[y * input->cols + x ]= std::min( p, 255 );
            }

        }
    }
}
}

ProcessThread::ProcessThread( sepia::Reader* inputGroup, sepia::Writer* outputGroup, boost::barrier *barrier, int id )
{
    m_inputGroup = inputGroup;
    m_outputGroup = outputGroup;
    m_barrier = barrier;
    m_id = id;
}

void ProcessThread::start()
{
    m_thread = new std::thread( std::bind( &ProcessThread::own_thread, this ) );
}

void ProcessThread::own_thread()
{
    int input_format = CV_8UC1;
    if( m_inputGroup->getHeader( m_id )->bpp == 24 )
    {
        input_format = CV_8UC3;
    }

    if( m_inputGroup->getHeader( m_id )->bpp == 32 )
    {
        input_format = CV_8UC4;
    }

    cv::Mat input_frame( m_inputGroup->getHeader( m_id )->height,
                         m_inputGroup->getHeader( m_id )->width,
                         input_format,
                         m_inputGroup->getAddress( m_id ) );

    int count = 0;
    auto begin = std::chrono::steady_clock::now();
    m_barrier->wait();

    std::cout << "input " << m_id << " bpp: " << m_inputGroup->getHeader( m_id )->bpp << std::endl;

    while( !m_terminate )
    {
        if( m_id == 0 )
        {
            if( count == 0 )
            {
                begin = std::chrono::steady_clock::now();
            }
            count++;
        }
        int output_format = CV_8UC3;
        if( Settings::use_raw || Settings::demosaicing_method == cv::COLOR_BayerBG2GRAY )
        {
            output_format = CV_8UC1;
        }

        cv::Mat output_frame( m_outputGroup->getHeader( m_id )->height, m_outputGroup->getHeader( m_id )->width, output_format, m_outputGroup->getAddress( m_id ) );

        if( input_frame.channels() == 1 )
        {
            //adjustWb( input_frame, input_image->info.Kr, input_image->info.Kg, input_image->info.Kb );
            if( Settings::use_raw )
            {
                memcpy( output_frame.data, input_frame.data, m_outputGroup->getHeader( m_id )->size );
            }
            else
            {
                cv::demosaicing( input_frame, output_frame, Settings::demosaicing_method );
            }
        }
        else if( input_frame.channels() == 4 )
        {
            cv::cvtColor( input_frame, output_frame, cv::COLOR_BGRA2BGR );
            //memcpy( output_frame->imageData, input_frame->imageData, output_image->info.size );
        }
        else
        {
            memcpy( output_frame.data, input_frame.data, m_outputGroup->getHeader( m_id )->size );
        }
        m_barrier->wait();
        if( m_id == 0 )
        {
            if( count == 100 )
            {
                auto end = std::chrono::steady_clock::now();
                auto elapsed = end-begin;
                //std::cerr << "Time: " << elapsed.count() * 1000.0 / std::chrono::steady_clock::period().den << std::endl;
                elapsed = end-begin;
                count = 0;
                std::cerr << "Process Fps: " << 100 * std::chrono::steady_clock::period().den / (double) elapsed.count() << std::endl;
                std::cerr << "Timestamp: " << m_inputGroup->getHeader( m_id )->tv_sec << "." << m_inputGroup->getHeader( m_id )->tv_usec << std::endl;
            }

            m_outputGroup->update();
            m_inputGroup->update();
        }
        m_barrier->wait();
        input_frame.data = reinterpret_cast< unsigned char* >( m_inputGroup->getAddress( m_id ) );
    }
}

