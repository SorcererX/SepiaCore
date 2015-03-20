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

#include "v4l2capture.h"
#include "v4l2camera.h"
#include <sepia/writer.h>

V4L2Capture::V4L2Capture( const int cameras )
{
    m_writer = new sepia::Writer( "/V4L2_IMAGE", cameras, 1280, 1024, 32 ); // reserve for 4 channels.
    m_barrier = new boost::barrier( cameras );

    std::string name = "/dev/video";
    for( int i = 0; i < cameras; i++ )
    {
        addCamera( name + std::to_string( i ) );
    }
    // FIXME: this is only suitable for Logitech C920
    int format = 1; // MJPEG
    int frame_width = 1600;
    int frame_height = 1200;
    for( int i = 0; i < cameras; i++ )
    {
        V4L2Camera* camera = m_cameras.at( i );
        camera->stopCapture();
        camera->setFormat( camera->getFormats().at( format ).pixelformat, frame_width, frame_height );
        camera->setPowerLineFrequencyTo50HZ();
        camera->setAutoExposure(); // for several synchronized cameras, this should be manual.
    }
}

V4L2Capture::~V4L2Capture()
{
}

void V4L2Capture::addCamera( const std::string& a_fileName )
{
    m_cameras.push_back( new V4L2Camera( a_fileName ) );
    m_acquisition.push_back( true ); // FIXME: set to false once V4L2Control is implemented.
}

bool V4L2Capture::isTerminated()
{
    return m_terminate;
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

   input->stopCapture();
   unsigned int frame_no = 0;

   input->init_mmap();
   input->readyCapture();
   m_barrier->wait();
   input->startCapture();

   bool ok = true;

   while( !m_terminate )
   {
       if( m_acquisition.at( camera_id ) )
       {
           sepia::Stream::image_header_t* hdr = m_writer->getHeader( camera_id );
           frame_no++;
           m_barrier->wait();
           ok = input->readyFrame();
           char* address = m_writer->getAddress( camera_id );
           struct timeval tv;
           input->readFrame( (unsigned char*) address, hdr->size, tv );
           hdr->fourcc = input->getCurrentFormat().fmt.pix.pixelformat;
           hdr->width = input->getCurrentFormat().fmt.pix.width;
           hdr->height = input->getCurrentFormat().fmt.pix.height;
           hdr->tv_sec = tv.tv_sec;
           hdr->tv_usec = tv.tv_usec;

           if( !ok )
           {
               std::cerr << "readyFrame failed." << std::endl;
               break;
           }

           if( camera_id == 0 )
           {
               m_writer->update();
           }
       }
       else
       {
           frame_no = 0;
           m_barrier->wait();
           usleep( 500000 ); // wait 500 ms before retrying.
       }
   }
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
