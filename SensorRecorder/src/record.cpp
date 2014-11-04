#include "record.h"
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <thread>
#include <sepia/reader.h>
#include <sys/time.h>

Record::Record( const std::string groupname, const std::string filename )
{
    m_shared = new sepia::Reader( groupname );
    m_fileList = filename;
}


void Record::start()
{
    m_thread = new std::thread( std::bind( &Record::own_thread, this ) );
}

void Record::own_thread()
{
    std::ofstream list;

    list.open( m_fileList );

    if( !list.is_open() )
    {
        std::cerr << "ERROR: " << "unable to open " << m_fileList << "." << std::endl;
        return ;
    }


    std::ofstream output;

    struct timeval tp;
    gettimeofday( &tp, NULL );

    std::stringstream ss;

    std::string basestr = std::to_string( tp.tv_sec );

    int counter = 0;

    while( !m_terminate )
    {
        m_shared->update();

        if( counter % 100 == 0 )
        {
            if( output.is_open() )
            {
                output.close();
            }
            ss.str("");
            ss.clear();
            ss << basestr;
            ss << "_";
            ss << std::setfill( '0' );
            ss << std::setw( 10 );
            ss << counter << ".bin";
            std::string filename = ss.str();

            output.open( filename, std::fstream::out | std::fstream::app );
            std::cout << std::endl << "Writing to: " << filename << " " << std::flush;
        }

        output.write( (char*) m_shared->getGroupHeader(), sizeof( sepia::Stream::group_header_t ) );

        for( size_t i = 0; i < m_shared->getGroupHeader()->count; i++ )
        {
            output.write( (char*) m_shared->getHeader( i ), sizeof( sepia::Stream::image_header_t ) );
        }
        for( size_t i = 0; i < m_shared->getGroupHeader()->count; i++ )
        {
            output.write( (char*) m_shared->getAddress( i ), m_shared->getHeader( i )->size );
        }


        std::cout << "." << std::flush;
        counter++;
    }
    if( output.is_open() )
    {
        output.close();
    }
}
