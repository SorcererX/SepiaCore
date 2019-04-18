#include <spinnakercapture.h>

#include <Spinnaker.h>
#undef interface

#include <boost/thread/barrier.hpp>

#include <sepia/writer.h>
#include <thread>


SpinnakerCapture::SpinnakerCapture( const std::string &a_outputName, const int a_cameras )
{
    m_barrier = new boost::barrier( a_cameras );

    m_writer = new sepia::Writer( a_outputName.c_str(), a_cameras, 2448, 2048, 24 );

    // do nothing.
    m_terminate = false;

    //sepia::comm::Observer< cuttlefish_msgs::FC2SetExposure >::initReceiver();
}

SpinnakerCapture::~SpinnakerCapture()
{
}

//void FC2Capture::receive( const cuttlefish_msgs::FC2SetExposure* a_msg )
//{
//    a_msg->exposure();
//}


void SpinnakerCapture::acquisition_thread( const int a_cameraNo )
{

    Spinnaker::SystemPtr system = Spinnaker::System::GetInstance();

    Spinnaker::CameraList list = system->GetCameras();

    Spinnaker::CameraPtr camera = list.GetByIndex( a_cameraNo );

    camera->Init();

    camera->BeginAcquisition();

    int frame_no = 0;

    Spinnaker::ImagePtr image;

    while( !m_terminate )
    {
        frame_no++;
        image = camera->GetNextImage();

        if( a_cameraNo == 0 )
        {
            std::cout << "." << std::flush;
            if( frame_no % 50 == 0 )
            {
                std::cout << " " << frame_no << std::endl;
            }
        }

        m_writer->setSize( a_cameraNo, image->GetWidth(), image->GetHeight(), image->GetBitsPerPixel() );
        sepia::Stream::image_header_t* hdr = m_writer->getHeader( a_cameraNo );

        hdr->tv_sec = image->GetTimeStamp();
        hdr->tv_usec = 0;
        hdr->frame_counter = image->GetFrameID();

        m_writer->copyWrite( a_cameraNo, reinterpret_cast< char* >( image->GetData() ) );

        m_barrier->wait();

        if( a_cameraNo == 0 )
        {
            m_writer->update();
        }
    }


    /*

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
    */

}

bool SpinnakerCapture::isTerminated()
{
    return m_terminate;
}

void SpinnakerCapture::start()
{
    Spinnaker::SystemPtr system = Spinnaker::System::GetInstance();

    Spinnaker::CameraList list = system->GetCameras();

    std::cout << "Spinnaker detected cameras: " << list.GetSize() << std::endl;

    for( unsigned int i = 0; i < list.GetSize(); i++ )
    {
        std::cout << "Create Spinnaker thread" << std::endl;
        m_threads.push_back( new std::thread( std::bind( &SpinnakerCapture::acquisition_thread, this, i ) ) );
    }
    //system->ReleaseInstance();
}

void SpinnakerCapture::stop()
{
    m_terminate = true;
}

void SpinnakerCapture::join()
{
    for( unsigned int i = 0; i < m_threads.size(); i++ )
    {
        m_threads[i]->join();
    }
}
