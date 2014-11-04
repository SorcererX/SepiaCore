#include "v4l2capture.h"
#include "v4l2camera.h"


V4L2Capture::V4L2Capture( const int cameras )
{
}

V4L2Capture::~V4L2Capture()
{
}

void V4L2Capture::addCamera( const std::string& a_fileName )
{
    m_cameras.push_back( new V4L2Camera( a_fileName ) );
    m_acquisition.push_back( false );
}

void V4L2Capture::start()
{
    for( unsigned int i = 0; i < m_cameras.size(); i++ )
    {
       m_threads.push_back( new boost::thread( boost::bind( &V4L2Capture::acquisition_thread, this, i ) ) );
    }
}

void V4L2Capture::stop()
{
    m_terminate = true;
}

void V4L2Capture::join()
{
    for( unsigned int i = 0; i < m_threads.size(); i++ )
    {
        m_threads[i]->join();
    }
}


void V4L2Capture::receive( const cuttlefish_msgs::V4L2Control* msg )
{
    const int camera = msg->camera_no();

    switch( msg->command() )
    {
    case cuttlefish_msgs::V4L2Control_CommandType_OPEN:
        break;
    case cuttlefish_msgs::V4L2Control_CommandType_CLOSE:
        break;
    case cuttlefish_msgs::V4L2Control_CommandType_START:
        break;
    case cuttlefish_msgs::V4L2Control_CommandType_STOP:
        break;
    case cuttlefish_msgs::V4L2Control_CommandType_TERMINATE:
        break;
    }
}


void V4L2Capture::acquisition_thread( const int camera_id )
{
   V4L2Camera* input = m_cameras.at( camera_id );
   /*
   input->stopCapture();
   sleep( 2 );

   int frame=0;
   input->init_mmap();
   input->readyCapture();

   barrier->wait();
   input->startCapture();

   image_t* image = data->localImage( id );
   image->info.width = settings::width;
   image->info.height = settings::height;
   image->info.Kb = 1.0;
   image->info.Kg = 1.0;
   image->info.Kr = 1.0;
   image->info.max_size = image->info.width * image->info.height * 2; // YUV = 2 bytes per pixel.
   image->data = new unsigned char[ image->info.max_size ];


   if( !( input->readyFrame() &&
          input->readFrame( image->data, image->info.size, image->info.timestamp )
          ) )
   {
      std::cerr << "Unable to read the first frame." << std::endl;
      return;
   }

   std::cerr << "Camera(" << id << ") - frame size: " << image->info.size << std::endl;

   bool ok = true;

   int accum_diff = 0; // only used by id == 0
   int accum_frames = 0; // only used by id == 0
   int accum_diff01 = 0; // only used by id == 0

   int default_exposure = 0;
   input->getAbsoluteExposure( default_exposure );

   while( true )
   {
      ok = input->readyFrame();

      if( !ok )
      {
         std::cerr << "readyFrame failed." << std::endl;
         break;
      }
      input->readFrame( (unsigned char*) image->data, image->info.size, image->info.timestamp );
      input->getAbsoluteExposure( image->info.exposure_time );

      if( image->info.exposure_time != default_exposure )
      {
         input->setAbsoluteExposure( default_exposure );
      }

      frame++;

      image->info.frame_number = frame;

      barrier->wait();

      if( id == 0 ) // only one thread executes this code.
      {
         int diff = 0;
         struct timeval min = image->info.timestamp;
         struct timeval max = image->info.timestamp;

         for( unsigned int i = 0; i < data->localGroupSize(); i++ )
         {
            struct timeval tv = m_outputData->localImage( i )->info.timestamp;

            if( timevaldiff( &min, &tv ) > 0 )
            {
               min = tv;
            }
            if( timevaldiff( &max, &tv ) < 0 )
            {
               max = tv;
            }
         }
         diff = abs( timevaldiff( &max, &min ) );
         int diff01 = timevaldiff( &m_outputData->localImage( 0 )->info.timestamp, &m_outputData->localImage( 1 )->info.timestamp );
         accum_diff01 += diff01;

         accum_diff += diff;
         accum_frames++;

         if( diff < 2500 )
         {
            data->writeAndNotify();
            std::cerr << "." << std::flush;
         }
         else
         {
            std::cerr << "\n" << "frame " << frame << " dropped - time delta: " << diff/1000.0 << " ms" << std::endl;
         }
         if( accum_frames > 5 )
         {
            if( ( accum_diff / accum_frames ) > 10000 )
            {
               m_needSync = true;
            }
            else
            {
               //int diff = accum_diff / ( accum_frames * 1000.0 ) + 0.5;

               if( accum_diff01 / accum_frames > 75 )
               {
                  diff = 1;
               }
               else if( accum_diff01 / accum_frames < -75 )
               {
                  diff = -1;
               }
               else
               {
                  diff = 0;
               }
               if( diff != 0 )
               {
                  input->setAbsoluteExposure( default_exposure + diff );
                  int value;
                  input->getAbsoluteExposure( value );
                  std::cout << "used exposure: " << value << std::endl;
                  accum_diff = 0;
                  accum_frames = 0;
                  accum_diff01 = 0;
               }
            }
         }
      }

      barrier->wait();
      if( m_needSync == true )
      {
         std::cerr << "sync required. - doing sync." << std::endl;
         input->stopCapture();
         input->readyCapture();
         barrier->wait();
         input->startCapture();

         if( id == 0 )
         {
            *need_sync = false;
            accum_diff = 0;
            accum_frames = 0;
            accum_diff01 = 0;
         }
      }

      if( id == 0 && accum_frames >= 50 )
      {
         std::cerr << "Avg: " << accum_diff / accum_frames / 1000.0 << " ms" << std::endl;
         accum_frames = 0;
         accum_diff = 0;
         accum_diff01 = 0;
      }
   }
   input->stopCapture();
   */
}
