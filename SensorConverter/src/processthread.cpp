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
