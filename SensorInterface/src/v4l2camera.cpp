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

#include "v4l2camera.h"
#include <libv4l2.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <v4l2interface.h>

V4L2Camera::V4L2Camera( std::string filename )
{
    m_deviceName = filename;
}



V4L2Camera::~V4L2Camera()
{
    stopCapture();
    close( m_fd );
}


int V4L2Camera::getFileDesc()
{
    return m_fd;
}


void V4L2Camera::open()
{
    V4L2Interface::open_device( m_fd, m_deviceName );
    V4L2Interface::query_capabilities( m_fd, m_deviceName );
    V4L2Interface::reset_cropcap( m_fd );
}

std::vector< v4l2_fmtdesc >& V4L2Camera::getFormats()
{
    if( m_formatList.size() == 0 )
    {
        V4L2Interface::enum_formats( m_fd, m_formatList );
    }
    return m_formatList;
}


std::vector< v4l2_frmsizeenum >& V4L2Camera::getFrameSizes( u_int32_t format )
{
    if( m_frameSizeList.count( format ) == 0 )
    {
        std::vector< v4l2_frmsizeenum > list;
        V4L2Interface::enum_framesizes( m_fd, format, list );
        m_frameSizeList[ format ] = list;
    }

    return m_frameSizeList[ format ];
}

std::vector< v4l2_frmivalenum >& V4L2Camera::getFrameIntervals( u_int32_t format, u_int32_t width, u_int32_t height )
{
    if( m_fpsList.count( format ) == 0 )
    {
        std::map< std::pair< u_int32_t, u_int32_t >, std::vector< v4l2_frmivalenum > > temp;
        m_fpsList[ format ] = temp;
    }

    if( m_fpsList[ format ].count( std::pair< u_int32_t, u_int32_t >( width, height ) ) == 0 )
    {
        std::vector< v4l2_frmivalenum > list;
        V4L2Interface::enum_frameintervals( m_fd, format, width, height, list );
        m_fpsList[ format ][ std::pair< u_int32_t, u_int32_t >( width, height ) ] = list;
    }
    return m_fpsList[ format ][ std::pair< u_int32_t, u_int32_t >( width, height ) ];
}


void V4L2Camera::setFormat( u_int32_t pixelformat, u_int32_t width, u_int32_t height )
{
    if( V4L2Interface::set_format( m_fd, pixelformat, width, height ) )
    {
        std::cout << "setting current format" << std::endl;
        if( V4L2Interface::get_format( m_fd, m_completeFormat ) )
        {
            std::cout << m_completeFormat.fmt.pix.width << " " << m_completeFormat.fmt.pix.height << " " << m_completeFormat.fmt.pix.pixelformat << std::endl;
        }
    }
}

struct v4l2_format V4L2Camera::getCurrentFormat()
{
    return m_completeFormat;
}


void V4L2Camera::init_mmap()
{
    int count = 4;
    V4L2Interface::request_buffers( m_fd, count );

    buffers = (V4L2Interface::buffer*) calloc ( count, sizeof (*buffers) );

    if (!buffers)
    {
        fprintf (stderr, "Out of memory\n");
        exit (EXIT_FAILURE);
    }

    for ( m_nBuffers = 0; m_nBuffers < count; ++m_nBuffers) {
        struct v4l2_buffer buf;
        V4L2Interface::query_buffer( m_fd, buf, m_nBuffers );
        V4L2Interface::memory_map( m_fd, buf, buffers[ m_nBuffers ] );
    }
}

void V4L2Camera::readyCapture()
{
    for ( unsigned int i = 0; i < m_nBuffers; ++i) {
        V4L2Interface::queue_buffer( m_fd, i );
    }
}

void V4L2Camera::startCapture()
{
    V4L2Interface::stream_on( m_fd );
}

void V4L2Camera::stopCapture()
{
    V4L2Interface::stream_off( m_fd );
}


bool V4L2Camera::readyFrame()
{
    int timeout = 2;
    return V4L2Interface::wait_for_data( m_fd, timeout );
}

bool V4L2Camera::readFrame( unsigned char* data, unsigned int &size, struct timeval& timestamp )
{
    struct v4l2_buffer buf;//needed for memory mapping

    bool retval;

    retval = V4L2Interface::dequeue_buffer( m_fd, buf );
    if( !retval ) return false;

    memcpy( data, buffers[buf.index].start, buf.bytesused );
    timestamp = buf.timestamp;

    size = buf.bytesused;

    retval = V4L2Interface::queue_buffer( m_fd, buf.index );
    return retval;
}


void V4L2Camera::printFormatList()
{
    for( uint i = 0; i < getFormats().size(); i++ )
    {
        std::cout << getFormats().at(i).description;
        if( getFormats().at(i).flags & V4L2_FMT_FLAG_EMULATED
          || getFormats().at(i).flags & V4L2_FMT_FLAG_COMPRESSED )
        {
            std::cout << " (";

            if( getFormats().at(i).flags & V4L2_FMT_FLAG_EMULATED )
            {
                std::cout << " EMULATED ";
            }
            if( getFormats().at(i).flags & V4L2_FMT_FLAG_COMPRESSED )
            {
                std::cout << " COMPRESSED ";
            }
            std::cout << ")";
        }
        std::cout << std::endl;

        u_int32_t pixel_format = getFormats().at( i ).pixelformat;
        for( uint j = 0; j < getFrameSizes( pixel_format ).size(); j++ )
        {
            u_int32_t width = getFrameSizes( pixel_format ).at( j ).discrete.width;
            u_int32_t height = getFrameSizes( pixel_format ).at( j ).discrete.height;
            std::cout << width << "x" << height << ": ";
            for( uint k = 0; k < getFrameIntervals( pixel_format, width, height ).size(); k++ )
            {
                u_int32_t denominator = getFrameIntervals( pixel_format, width, height ).at( k ).discrete.denominator;
                u_int32_t numerator = getFrameIntervals( pixel_format, width, height ).at( k ).discrete.numerator;
                std::cout << denominator/numerator << ", ";
            }
            std::cout << std::endl;
        }
    }
}

bool V4L2Camera::setAbsoluteExposure( int msec )
{
    return V4L2Interface::set_ctrl( m_fd, V4L2_CID_EXPOSURE_ABSOLUTE, msec );
}

bool V4L2Camera::getAbsoluteExposure( int& msec )
{
    return V4L2Interface::get_ctrl( m_fd, V4L2_CID_EXPOSURE_ABSOLUTE, msec );
}

bool V4L2Camera::setPowerLineFrequencyTo50HZ()
{
    return V4L2Interface::set_ctrl( m_fd, V4L2_CID_POWER_LINE_FREQUENCY, V4L2_CID_POWER_LINE_FREQUENCY_50HZ );
}

bool V4L2Camera::setManualExposure()
{
    return V4L2Interface::set_ctrl( m_fd, V4L2_CID_EXPOSURE_AUTO, V4L2_EXPOSURE_MANUAL );
}

bool V4L2Camera::setAutoExposure()
{
    return V4L2Interface::set_ctrl( m_fd, V4L2_CID_EXPOSURE_AUTO, V4L2_EXPOSURE_AUTO );
}

bool V4L2Camera::setGain( int value )
{
    return V4L2Interface::set_ctrl( m_fd, V4L2_CID_GAIN, value );
}

bool V4L2Camera::getGain( int& value )
{
    return V4L2Interface::get_ctrl( m_fd, V4L2_CID_GAIN, value );
}

