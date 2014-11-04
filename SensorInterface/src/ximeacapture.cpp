#include "ximeacapture.h"
#include <boost/thread/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <functional>
#include <chrono>
#include <sepia/comm/dispatcher.h>
#include <sepia/writer.h>
#include <string>
#include <usbreset.h>

using namespace cuttlefish_msgs;


namespace
{
    std::string errorString( int errorcode )
    {
        static std::map< int, std::string > sm_map;

        if( sm_map.empty() )
        {
            sm_map[ XI_OK ] = "OK"; // 0
            sm_map[ XI_WRITEREG ] = "Register Write Error"; // 3
            sm_map[ XI_TIMEOUT  ] = "Timeout"; // 10
            sm_map[ XI_ISOCH_ATTACH_BUFFERS ] = "Attach buffers error"; // 13
            sm_map[ XI_ACQUISITION_STOPED ] = "Acquisition Stopped"; // 45
            sm_map[ XI_NO_DEVICES_FOUND   ] = "No Devices Found"; // 56
        }
        std::map< int, std::string>::iterator it = sm_map.find( errorcode );
        if( it != sm_map.end() )
        {
            return it->second;
        }
        else
        {
            return std::string( "unknown" );
        }
    }

    void printError( int errorcode )
    {
        std::cout << errorString( errorcode ) << " (" << errorcode << ")" << std::endl;
    }
}








XimeaCapture::XimeaCapture( const int devices )
{
    // Reset
    UsbReset r;

    // add XIMEA Vendor:Product IDs
    r.addId( 0x04B4, 0x00F0 );
    r.addId( 0x04B4, 0x00F1 );
    r.addId( 0x04B4, 0x00F2 );
    r.addId( 0x04B4, 0x00F3 );
    r.addId( 0x04B4, 0x8613 );

    r.addId( 0x20F7, 0x3000 );
    r.addId( 0x20F7, 0x3001 );
    r.addId( 0x20F7, 0xA003 );

    r.addId( 0xDEDA, 0xA003 );
    r.reset();

    m_terminate = false;

    m_writer = new sepia::Writer( "/XI_IMG", devices, 1280, 1024, 32 ); // reserve for 4 channels.
    for( int i = 0; i < devices; i++ )
    {
        m_writer->setSize( i, 1280, 1024, 8 );
    }

    m_handles.assign( devices, NULL );
    m_acquisition.assign( devices, false );
    m_trigger.assign( devices, false );
    m_barrier = new boost::barrier( devices );
    m_triggerBarrier = new boost::barrier( devices );
    xiSetParamInt( 0, XI_PRM_DEBUG_LEVEL, XI_DL_WARNING );
    xiSetParamInt( 0, XI_PRM_AUTO_BANDWIDTH_CALCULATION, XI_OFF );
}

XimeaCapture::~XimeaCapture()
{
    stop();
    join();
}

void XimeaCapture::start()
{
   for( unsigned int i = 0; i < m_handles.size(); i++ )
   {
      m_threads.push_back( new boost::thread( boost::bind( &XimeaCapture::acquisition_thread, this, i ) ) );
   }
}

void XimeaCapture::stop()
{
    m_terminate = true;
}

void XimeaCapture::join()
{
    for( unsigned int i = 0; i < m_threads.size(); i++ )
    {
        m_threads[i]->join();
    }
}

void XimeaCapture::receive( const XimeaControl* msg )
{
   const int camera_no = msg->camera_no();
   int retval = 0;
   int temp = 0;
   switch( msg->command() )
   {
      case XimeaControl_CommandType_OPEN:
         if( camera_no == 1 )
         {
             temp = 1;
         }
         retval = xiOpenDevice( temp, &m_handles.at( camera_no ) );
         if( m_handles.at( camera_no ) == NULL )
         {
             std::cerr << "OPEN FAILED.";
             stop();
         }
         break;
      case XimeaControl_CommandType_CLOSE:
         retval = xiCloseDevice( m_handles.at( camera_no ) );
         break;
      case XimeaControl_CommandType_START:
         retval = xiStartAcquisition( m_handles.at( camera_no ) );
         if( retval == 0 )
         {
            m_acquisition.at( camera_no ) = true;
         }
         else
         {
             xiStopAcquisition( m_handles.at( camera_no ) );
         }
         break;
      case XimeaControl_CommandType_STOP:
         m_acquisition.at( camera_no ) = false;
         retval = xiStopAcquisition( m_handles.at( camera_no ) );
         break;
      case XimeaControl_CommandType_TERMINATE:
         std::cout << "GOT TERMINATION COMMAND." << std::endl;
         stop();
         break;
      case XimeaControl_CommandType_TRIGGER_START:
         m_trigger.at( camera_no ) = true;
         break;
      case XimeaControl_CommandType_TRIGGER_STOP:
         m_trigger.at( camera_no ) = false;
         break;
   }
   printError( retval );
}

void XimeaCapture::receive( const cuttlefish_msgs::XimeaSet* msg )
{
    const int camera_no = msg->camera_no();

    int retval = 0;

    if( msg->has_int_value() )
    {
        retval = xiSetParamInt( m_handles.at( camera_no ), msg->parameter().c_str(), msg->int_value() );
    }
    else if( msg->has_float_value() )
    {
        retval = xiSetParamFloat( m_handles.at( camera_no ), msg->parameter().c_str(), msg->float_value() );
    }
    printError( retval );
}

void XimeaCapture::receive( const cuttlefish_msgs::XimeaGet* msg )
{
    const int camera_no = msg->camera_no();
    cuttlefish_msgs::XimeaReply sMsg;
    sMsg.mutable_request()->set_camera_no( msg->camera_no() );
    sMsg.mutable_request()->set_parameter( msg->parameter() );
    sMsg.mutable_request()->set_type( msg->type() );

    int32_t int_value = 0;
    float float_value = 0;

    switch( msg->type() )
    {
        case XimeaGet_Type_INT:
           xiGetParamInt( m_handles.at( camera_no ), msg->parameter().c_str(), &int_value );
           sMsg.set_int_value( int_value );
           sepia::comm::Dispatcher<  cuttlefish_msgs::XimeaReply >::send( &sMsg );
           break;
        case XimeaGet_Type_FLOAT:
           xiGetParamFloat( m_handles.at( camera_no ), msg->parameter().c_str(), &float_value );
           sMsg.set_float_value( float_value );
           sepia::comm::Dispatcher<  cuttlefish_msgs::XimeaReply >::send( &sMsg );
           break;
    }
}

void XimeaCapture::acquisition_thread( const int camera_no )
{
    XI_IMG xi_image;

    xi_image.size = sizeof(XI_IMG);
    xi_image.bp = NULL;
    xi_image.bp_size = 0;

    unsigned int frame_no = 0;

    m_barrier->wait();
    auto start = std::chrono::steady_clock::now();

    while( !m_terminate )
    {
        if( m_acquisition.at( camera_no ) )
        {
           sepia::Stream::image_header_t* hdr = m_writer->getHeader( camera_no );
           frame_no++;
           m_barrier->wait();
           xi_image.bp = m_writer->getAddress( camera_no );
           xi_image.bp_size = hdr->size;

           int retval = xiGetImage( m_handles.at( camera_no ), 1000, &xi_image );

           if( retval != 0 )
           {
               printError( retval );
               m_barrier->wait();
               continue;
           }

           hdr->width = xi_image.width;
           hdr->height = xi_image.height;
           hdr->frame_number = xi_image.nframe;
           hdr->tv_sec = xi_image.tsSec;
           hdr->tv_usec = xi_image.tsUSec;
           hdr->fourcc = 1;
           hdr->size = xi_image.bp_size;
           hdr->gpi_level = xi_image.GPI_level;
           hdr->black_level = xi_image.black_level;

           m_barrier->wait();

           if( camera_no == 0 )
           {
               std::cerr << "." << std::flush;
               m_writer->update();
               if( frame_no % 50 == 0 )
               {
                  auto end = std::chrono::steady_clock::now();
                  auto elapsed = end - start;
                  std::cerr << " Avg FPS: " << 50.0 * std::chrono::steady_clock::period().den / (double) elapsed.count() << std::endl;
                  start = end;
               }
           }
        }
        else
        {
           frame_no = 0;
           m_barrier->wait();
           usleep( 500000 ); // wait 500 ms before retrying.
        }
    }
}


bool XimeaCapture::isTerminated()
{
    return m_terminate;
}
