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
#include "grfmt_jpeg.h"

#define FOURCC(a,b,c,d) ( (uint32_t) (((d)<<24) | ((c)<<16) | ((b)<<8) | (a)) )

ProcessThread::ProcessThread( sepia::Reader* input, sepia::Writer* output, boost::barrier *barrier, int id )
{
    m_input = input;
    m_output = output;
    m_rectifier = NULL;
    m_barrier = barrier;
    m_id = id;
}

void ProcessThread::start()
{
    m_thread = new std::thread( std::bind( &ProcessThread::own_thread, this ) );
}

void ProcessThread::setRectification( Rectification* a_rectifier )
{
    m_rectifier = a_rectifier;
}

void ProcessThread::own_thread()
{
    enum class Format { UNKNOWN, RAW8, RAW16, MJPEG, YUYV, BGR8 };

    Format format = Format::UNKNOWN;
    int cv_format = CV_MAKETYPE( CV_8U, 1 );

    sepia::Stream::image_header_t* hdr = m_input->getHeader( m_id );
    switch( hdr->fourcc )
    {
    case 0x00000000:
        if( hdr->bpp == 8 )
        {
            format = Format::RAW8;
            cv_format = CV_MAKETYPE( CV_8U, 1 );
        }
        else if( hdr->bpp == 16 )
        {
            format = Format::RAW16;
            cv_format = CV_MAKETYPE( CV_16U, 1 );
        }
        else if( hdr->bpp == 24 )
        {
            format = Format::BGR8;
            cv_format = CV_MAKETYPE( CV_8U, 3 );
        }
        break;
    case FOURCC( 'M', 'J', 'P', 'G'):
        format = Format::MJPEG;
        cv_format = CV_MAKETYPE( CV_8U, 3 ); // format after conversion
        break;
    default:
        break;
    }

    cv::Mat input_frame( m_input->getHeader( m_id )->height, m_input->getHeader( m_id )->width, cv_format, m_input->getAddress( m_id ) );

    cv::Mat converted_frame( m_output->getHeader( m_id )->height, m_output->getHeader( m_id )->width, CV_8UC3 );

    if( m_rectifier == NULL )
    {
        converted_frame.data = reinterpret_cast< unsigned char* >( m_output->getAddress( m_id ) );
    }

    cv::Mat rectified_frame( m_output->getHeader( m_id )->height, m_output->getHeader( m_id )->width, CV_8UC3, m_output->getAddress( m_id ) );

    JpegDecoder decoder;

    while( !m_terminate )
    {
        if( format == Format::RAW8 || format == Format::RAW16 )
        {
            cv::demosaicing( input_frame, converted_frame, cv::COLOR_BayerBG2BGR_EA );
        }
        else if( format == Format::MJPEG )
        {
            // perform JPEG decode here
            decoder.readHeader( reinterpret_cast< unsigned char* >( input_frame.data ), m_input->getHeader( m_id )->size );
            decoder.readData( reinterpret_cast< unsigned char* >( converted_frame.data ), m_input->getHeader( m_id )->width * 3, true );
        }
        else {

        }

        if( m_rectifier != NULL )
        {
            if( m_id == 0 )
            {
                m_rectifier->remapLeft( &converted_frame, &rectified_frame );
            }
            else if( m_id == 1 )
            {
                m_rectifier->remapRight( &converted_frame, &rectified_frame );
            }
        }

        m_barrier->wait();
        if( m_id == 0 )
        {
            m_output->update();
            m_input->update();
        }
        m_barrier->wait();
        input_frame.data = reinterpret_cast< unsigned char* >( m_input->getAddress( m_id ) );

        if( m_rectifier != NULL )
        {
            rectified_frame.data = reinterpret_cast< unsigned char* >( m_output->getAddress( m_id ) );
        }
        else
        {
            converted_frame.data = reinterpret_cast< unsigned char* >( m_output->getAddress( m_id ) );
        }
    }
}
