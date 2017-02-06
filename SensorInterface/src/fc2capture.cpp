#include <fc2capture.h>
#include <boost/thread/barrier.hpp>
#include <flycapture/FlyCapture2.h>

#include <sepia/writer.h>
#include <thread>

FC2Capture::FC2Capture( const std::string &a_outputName, const int a_cameras )
{
    m_barrier = new boost::barrier( a_cameras );

    m_writer = new sepia::Writer( a_outputName.c_str(), a_cameras, 2448, 2048, 8 );
    // do nothing.
    m_terminate = false;
    m_handles.assign( a_cameras, NULL );

    //sepia::comm::Observer< cuttlefish_msgs::FC2SetExposure >::initReceiver();
}

FC2Capture::~FC2Capture()
{
}

void FC2Capture::receive( const cuttlefish_msgs::FC2SetExposure* a_msg )
{
    a_msg->exposure();
}


void FC2Capture::acquisition_thread( const int a_cameraNo )
{
    FlyCapture2::Error error;

    // find cameras
    FlyCapture2::BusManager busmgr;

    unsigned int cameras;

    error = busmgr.GetNumOfCameras( &cameras );

    if( error != FlyCapture2::PGRERROR_OK )
    {
        error.PrintErrorTrace();
        return ;
    }

    // get camera by index
    FlyCapture2::PGRGuid guid;

    error = busmgr.GetCameraFromIndex( a_cameraNo, &guid );

    if( error != FlyCapture2::PGRERROR_OK )
    {
        error.PrintErrorTrace();
        return ;
    }

    FlyCapture2::Camera camera;

    error = camera.Connect( &guid );

    if( error != FlyCapture2::PGRERROR_OK )
    {
        error.PrintErrorTrace();
        return ;
    }

    //
    FlyCapture2::FC2Config config;
    error = camera.GetConfiguration( &config );

    if( error != FlyCapture2::PGRERROR_OK )
    {
        error.PrintErrorTrace();
        return ;
    }

    error = camera.SetConfiguration( &config );

    if( error != FlyCapture2::PGRERROR_OK )
    {
        error.PrintErrorTrace();
        return ;
    }


    // start capture
    error = camera.StartCapture();

    if( error != FlyCapture2::PGRERROR_OK )
    {
        error.PrintErrorTrace();
        return ;
    }

    FlyCapture2::Image rawImage;
    FlyCapture2::ImageMetadata metadata;

    int frame_no = 0;

    while( !m_terminate )
    {
        frame_no++;
        error = camera.RetrieveBuffer( &rawImage );

        if( error != FlyCapture2::PGRERROR_OK )
        {
            error.PrintErrorTrace();
            return ;
        }

        if( a_cameraNo == 0 )
        {
            std::cout << "." << std::flush;
            if( frame_no % 50 == 0 )
            {
                std::cout << " " << frame_no << std::endl;
            }
        }
        metadata = rawImage.GetMetadata();

        m_writer->setSize( a_cameraNo, rawImage.GetCols(), rawImage.GetRows(), rawImage.GetBitsPerPixel() );
        sepia::Stream::image_header_t* hdr = m_writer->getHeader( a_cameraNo );

        hdr->tv_sec = rawImage.GetTimeStamp().seconds;
        hdr->tv_usec = rawImage.GetTimeStamp().microSeconds;
        hdr->frame_counter = metadata.embeddedFrameCounter;

//        FlyCapture2::CameraStats stats;
//        camera.GetStats( &stats );

        hdr->frame_number = frame_no;

        m_writer->copyWrite( a_cameraNo, reinterpret_cast< char* >( rawImage.GetData() ) );

        m_barrier->wait();
        if( a_cameraNo == 0 )
        {
            m_writer->update();
        }
    }

}

bool FC2Capture::isTerminated()
{
    return m_terminate;
}

void FC2Capture::start()
{
    for( unsigned int i = 0; i < m_handles.size(); i++ )
    {
       m_threads.push_back( new std::thread( std::bind( &FC2Capture::acquisition_thread, this, i ) ) );
    }
}

void FC2Capture::stop()
{
    m_terminate = true;
}

void FC2Capture::join()
{
    for( unsigned int i = 0; i < m_threads.size(); i++ )
    {
        m_threads[i]->join();
    }
}
