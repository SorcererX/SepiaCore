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

#include "playback.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <thread>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include <cmath>
#include <string.h>
#include <functional>

double timevaldiff( struct timeval* a, struct timeval* b)
{
    double a_val = static_cast< double >( a->tv_sec * 1000000.0 + a->tv_usec );
    double b_val = static_cast< double >( b->tv_sec * 1000000.0 + b->tv_usec );
    return fabs( a_val - b_val );
}

Playback::Playback( const std::string& groupname, const std::string& file_list )
{
    m_fps = 15.0;
    m_useRealFps = false;
    m_loop = false;
    m_groupName = groupname;
    m_recFileList = file_list;
}

void Playback::setFps( double fps )
{
    m_fps = fps;
}

void Playback::useRealFps( bool enable )
{
    m_useRealFps = enable;
}

void Playback::enableLoop( bool enable )
{
    m_loop = enable;
}

bool Playback::readImages( std::ifstream& input, sepia::Writer*& output )
{
    sepia::Stream::group_header_t group_header;

    if( !input.read( (char*) &group_header, sizeof( sepia::Stream::group_header_t ) ) )
    {
        return false;
    }

    for( size_t i = 0; i < group_header.count; i++ )
    {
        sepia::Stream::image_header_t hdr;

        if( !input.read( (char*) &hdr, sizeof( sepia::Stream::image_header_t ) ) )
        {
            return false;
        }

        if( output == NULL )
        {
            std::cout << "sepia::Writer params: ";
            std::cout << "groupName: " << m_groupName << " count: " << group_header.count;
            std::cout << " width: " << hdr.width << " height: " << hdr.height << " bpp: " << hdr.bpp << std::endl;
            output = new sepia::Writer( m_groupName, group_header.count, hdr.width, hdr.height, hdr.bpp );
        }
        memcpy( (char*) output->getHeader( i ), &hdr, sizeof( sepia::Stream::image_header_t ) );
    }

    for( size_t i = 0; i < output->getGroupHeader()->count; i++ )
    {
        if( !input.read( (char*) output->getAddress( i ), output->getHeader( i )->size ) )
        {
            return false;
        }
    }

    return true;
}


void Playback::own_thread()
{
    sepia::Writer* output = NULL;

    std::ifstream reclist;

    std::ifstream input;

    reclist.open( m_recFileList );

    if( !reclist.is_open() )
    {
        std::cerr << "ERROR: not able to open " << m_recFileList << "." << std::endl;
        return ;
    }

    struct timeval real_clock;

    struct timeval diff = { 0, 0 };
    struct timeval wait_time;

    while( !m_terminate )
    {
        if( reclist.eof() )
        {
            if( m_loop )
            {
                reclist.clear();
                reclist.seekg( 0, std::ios::beg );
            }
            else
            {
                reclist.close();
                break;
            }
        }
        std::string filename;
        getline( reclist, filename );

        if( !reclist.good() )
        {
            if( !m_loop )
            {
                break;
            }
            else
            {
                reclist.clear();
                reclist.seekg( 0, std::ios::beg );
            }
        }
        input.open( filename );

        if( !input.is_open() )
        {
            std::cerr << "Failed to open: " << filename << std::endl;
            continue;
        }
        std::cout << "Playing " << filename << std::endl;

        while( !input.eof() && readImages( input, output ) )
        {
            if( output == NULL )
            {
                std::cerr << "Unable to create SepiaStream writer" << std::endl;
                break;
            }
            gettimeofday( &real_clock, NULL );
            struct timeval stream_clock;
            stream_clock.tv_sec = output->getHeader( 0 )->tv_sec;
            stream_clock.tv_usec = output->getHeader( 0 )->tv_usec;
            if( diff.tv_sec == 0 && diff.tv_usec == 0 )
            {
                diff.tv_sec = real_clock.tv_sec-stream_clock.tv_sec;
                diff.tv_usec = real_clock.tv_usec-stream_clock.tv_usec;
                std::cout << "Time difference between stream and real clock: " << (int) diff.tv_sec << " seconds." << std::endl;
            }
            else
            {
                wait_time.tv_sec = stream_clock.tv_sec - ( real_clock.tv_sec - diff.tv_sec );
                wait_time.tv_usec = stream_clock.tv_usec - ( real_clock.tv_usec - diff.tv_usec );
                if( m_useRealFps )
                {
                    usleep( wait_time.tv_sec * 1000000 + wait_time.tv_usec );
                }
                else
                {
                    usleep( 1000000 / m_fps );
                }
            }
            output->update();
        }
        input.close();
    }
}
