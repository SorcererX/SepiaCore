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
void adjustWb( IplImage* input, double Kr, double Kg, double Kb )
{
    unsigned char* image = (unsigned char*) input->imageData;
    int p = 0;
    for( int y = 0; y < input->height; y++ )
    {
        if( ( y & 1 ) == 0)
        {
            for( int x = 0; x < input->width; x++ )
            {
                if( ( x & 1 ) == 0 )
                {
                    p = image[y * input->width + x ] * Kr;
                }
                else
                {
                    p = image[y * input->width + x ] * Kg;
                }
                image[y * input->width + x ]= std::min( p, 255 );
            }
        }
        else
        {
            for( int x = 0; x < input->width; x++ )
            {
                if( ( x & 1 )  == 1 )
                {
                    p = image[y * input->width + x ] * Kb;
                }
                else
                {
                    p = image[y * input->width + x ] * Kg;
                }
                image[y * input->width + x ]= std::min( p, 255 );
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
                std::cout << "black_level - 0: " << m_inputGroup->getHeader( 0 )->black_level
                          << " 1: " << m_inputGroup->getHeader( 1 )->black_level;
                auto elapsed = end-begin;
                //std::cerr << "Time: " << elapsed.count() * 1000.0 / std::chrono::steady_clock::period().den << std::endl;
                elapsed = end-begin;
                count = 0;
                std::cerr << "Process Fps: " << 100 * std::chrono::steady_clock::period().den / (double) elapsed.count() << std::endl;
            }

            m_outputGroup->update();
            m_inputGroup->update();
        }
        m_barrier->wait();
        input_frame.data = reinterpret_cast< unsigned char* >( m_inputGroup->getAddress( m_id ) );
    }
}

