#include <araviscapture.h>
#include <sepia/util/threadbarrier.h>
#include <sepia/writer.h>
#include <sensorinterface.h>
#include <cstdint>
#include <cstring>
#include <iostream>

#include <araviscamera.h>
#include <aravisimage.h>

AravisCapture::AravisCapture( std::size_t a_cameras ) : m_cameras( a_cameras )
{
    m_barrier = new sepia::util::ThreadBarrier( a_cameras );

}

AravisCapture::~AravisCapture()
{
}

void AravisCapture::stop()
{
    m_terminate = true;
}

void AravisCapture::join()
{
    for( auto& thread : m_threads )
    {
        thread->join();
    }
}
void AravisCapture::start()
{
    std::vector< std::string > cameras = AravisCamera::getList();

    for( auto& camera : cameras )
    {
        std::cout << camera << std::endl;
    }

    for( std::size_t i = 0; i < m_cameras && i < cameras.size(); i++ )
    {
        std::string deviceString = cameras[ i ];
        std::unique_ptr< std::thread > thr = std::make_unique< std::thread >( [this, i, deviceString]{ acquisition_thread( i, deviceString ); } );
        m_threads.push_back( std::move( thr ) );
    }
}

void AravisCapture::acquisition_thread( std::size_t a_id, const std::string& a_deviceString )
{
    AravisCamera camera( a_deviceString );

    int64_t width = camera.getInt( "Width" );
    int64_t height = camera.getInt( "Height" );
    if( a_id == 0 )
    {
        std::cout << "Using device: " << a_deviceString << " width: " << width << " height: " << height << std::endl;
        m_writer = new sepia::Writer( "ARAVIS", m_cameras, width, height, 8 ); // reserve for 4 channels.
    }
    m_barrier->wait();

    std::unique_ptr< AravisStream > stream = camera.getStream();
    camera.startAcquisition();

    uint64_t counter = 0;
    while( !m_terminate )
    {
        std::unique_ptr< AravisImage > image = stream->getImage();

        if( !image->isValid() )
        {
            continue;
        }

        std::memcpy( m_writer->getAddress( a_id ), image->getData(), image->getSize() );

        if( a_id == 0 )
        {
            m_writer->update();
        }

        counter++;
        std::cout << "." << std::flush;

        if( counter % 100 == 0 )
        {
            std::cout << " " << counter << std::endl;
        }

    }
    camera.stopAcquisition();
    delete m_writer;
    m_writer = nullptr;
}

bool AravisCapture::isTerminated()
{
}
