#include "playback.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <thread>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>

int64_t timevaldiff(struct timeval *starttime, struct timeval *finishtime)
{
  int64_t usec;
  usec=(finishtime->tv_sec-starttime->tv_sec)*1000*1000;
  usec+=(finishtime->tv_usec-starttime->tv_usec);
  return usec;
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


void Playback::start()
{
    m_thread = new std::thread( std::bind( &Playback::own_thread, this ) );
}

bool Playback::readImages( std::ifstream& input, sepia::Writer& output )
{
    sepia::Stream::group_header_t group_header;

    if( !input.read( (char*) &group_header, sizeof( sepia::Stream::group_header_t ) ) )
    {
        return false;
    }

    if( group_header.count != output.getGroupHeader()->count )
    {
        std::cerr << "Group size does not match." << std::endl;
        return false;
    }

    for( size_t i = 0; i < output.getGroupHeader()->count; i++ )
    {
        if( !input.read( (char*) output.getHeader( i ), sizeof( sepia::Stream::image_header_t ) ) )
        {
            return false;
        }
    }
    for( size_t i = 0; i < output.getGroupHeader()->count; i++ )
    {
        if( !input.read( (char*) output.getAddress( i ), output.getHeader( i )->size ) )
        {
            return false;
        }
    }

    return true;
}


void Playback::own_thread()
{
    sepia::Writer output( m_groupName, 2, 1280, 1024, 8 ); // temporary, need to sort out what to do.

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
            gettimeofday( &real_clock, NULL );
            struct timeval stream_clock;
            stream_clock.tv_sec = output.getHeader( 0 )->tv_sec;
            stream_clock.tv_usec = output.getHeader( 0 )->tv_usec;
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
            output.update();
        }
        input.close();
    }
}
