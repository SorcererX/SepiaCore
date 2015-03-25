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
#include "rectification.h"
#include <sepia/reader.h>
#include <sepia/writer.h>

ProcessThread::ProcessThread( sepia::Reader* input, sepia::Writer* output, Rectification* rectifier, boost::barrier *barrier, int id )
{
    m_input = input;
    m_output = output;
    m_rectifier = rectifier;
    m_barrier = barrier;
    m_id = id;
}

void ProcessThread::start()
{
    m_thread = new std::thread( std::bind( &ProcessThread::own_thread, this ) );
}

void ProcessThread::own_thread()
{
    int format = CV_8UC1; // default
    if( m_input->getHeader( m_id )->bpp == 8 )
    {
        format = CV_8UC1;
    }
    else if( m_input->getHeader( m_id )->bpp == 24 )
    {
        format = CV_8UC3;
    }

    cv::Mat input_frame( m_input->getHeader( m_id )->height, m_input->getHeader( m_id )->width, format, m_input->getAddress( m_id ) );
    cv::Mat temp_frame( m_output->getHeader( m_id )->height, m_output->getHeader( m_id )->width, CV_8UC3 );
    cv::Mat output_frame( m_output->getHeader( m_id )->height, m_output->getHeader( m_id )->width, CV_8UC3, m_output->getAddress( m_id ) );

    while( !m_terminate )
    {
        if( format == CV_8UC1 )
        {
            cv::demosaicing( input_frame, temp_frame, cv::COLOR_BayerBG2BGR_EA );
            if( m_id == 1 )
            {
                //temp_frame /= 0.9;
            }
        }
        if( m_id == 0 )
        {
            m_rectifier->remapLeft( &temp_frame, &output_frame );
        }
        else if( m_id == 1 )
        {
            m_rectifier->remapRight( &temp_frame, &output_frame );
        }
        m_barrier->wait();
        if( m_id == 0 )
        {
            m_output->update();
            m_input->update();
        }
        m_barrier->wait();
        input_frame.data = reinterpret_cast< unsigned char* >( m_input->getAddress( m_id ) );
        output_frame.data = reinterpret_cast< unsigned char* >( m_output->getAddress( m_id ) );
    }
}
