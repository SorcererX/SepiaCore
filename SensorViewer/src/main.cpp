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

#include <GL/gl.h>
#include <GL/glext.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <sstream>
#include <opencv2/opencv.hpp>
//#include <opencv2/legacy/compat.hpp>
#include <cmath>
#include <processthread.h>
#include <settings.h>
#include <boost/program_options.hpp>

#include <sepia/writer.h>
#include <sepia/reader.h>

namespace po = boost::program_options;

//unsigned int window_width = 1920, window_height = 1024;
//unsigned int window_width = 2560, window_height = 1024;
//unsigned int window_width = 1280, window_height=1024;

void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    switch( key )
    {
    case GLFW_KEY_1:
        Settings::use_raw = false;
        std::cout << "demosaic: EA" << std::endl;
        Settings::demosaicing_method = cv::COLOR_BayerBG2BGR_EA;
        break;
    case GLFW_KEY_2:
        Settings::use_raw = false;
        std::cout << "demosaic: VNG" << std::endl;
        Settings::demosaicing_method = cv::COLOR_BayerBG2BGR_VNG;
        break;
    case GLFW_KEY_3:
        Settings::use_raw = false;
        std::cout << "demosaic: grey" << std::endl;
        Settings::demosaicing_method = cv::COLOR_BayerBG2GRAY;
        break;
    case GLFW_KEY_4:
        Settings::use_raw = true;
        Settings::demosaicing_method = cv::COLOR_BayerBG2GRAY;
        break;
    case GLFW_KEY_5:
        Settings::show_mode = Settings::show::BOTH;
        break;
    case GLFW_KEY_6:
        Settings::show_mode = Settings::show::LEFT;
        break;
    case GLFW_KEY_7:
        Settings::show_mode = Settings::show::RIGHT;
        break;
    case GLFW_KEY_8:
        Settings::use_raw = false;
        Settings::demosaicing_method = cv::COLOR_BayerBG2BGR_EA;
        Settings::show_mode = Settings::show::STEREO;
        break;
    case GLFW_KEY_F:
        Settings::force_noscale = !Settings::force_noscale;
        break;
    case GLFW_KEY_Q:
        glfwSetWindowShouldClose(window, GL_TRUE);
        break;
    default:
        break;
    }
}

static void focus_callback( GLFWwindow* window, int focused )
{

}

// this simply combines the left and right image into an anaglyphic stereo image.
// supports B&W and RGB images.
void anaglyphic_stereo(cv::Mat left_input, cv::Mat right_input, cv::Mat output)
{
        for (int y=0; y<left_input.size().height; y++)
        {
            const unsigned char* left_row = left_input.ptr< unsigned char >( y );
            const unsigned char* right_row = right_input.ptr< unsigned char >( y );
            unsigned char* output_row = output.ptr< unsigned char >( y );
            for (int x=0; x<left_input.size().width * 3; x+= 3)
            {
                // BGR
                output_row[ x   ] = cv::saturate_cast<unsigned char>(  left_row[ x   ] * 1.1 ); // B
                output_row[ x+1 ] = cv::saturate_cast<unsigned char>(  left_row[ x+1 ] * 1.1 ); // G
                output_row[ x+2 ] = cv::saturate_cast<unsigned char>( right_row[ x+2 ] * 1.1 ); // R
            }
        }
}

int main(int argc, char** argv)
{
    glfwSetErrorCallback(error_callback);

    po::options_description desc;

    std::string input_name;

    desc.add_options()
            ( "input_name",po::value<std::string>(&input_name)->default_value("XI_IMG"), "Input Group Name" );

    po::variables_map vm;

    try
    {
        po::store( po::parse_command_line( argc, argv, desc ), vm );
        po::notify( vm );
    }
    catch( const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        std::cout << desc << std::endl;
        return 1;
    }

    if( vm.count( "input_name" ) )
    {
        std::cout << "Using: " << input_name << std::endl;
    }



    int window_width = 1792, window_height = 768;
    GLFWwindow* window;

    if( !glfwInit() )
    {
        std::cout << "glfwInit failed." << std::endl;
        exit( -1 );
    }

    window = glfwCreateWindow( window_width, window_height, "SensorGL", NULL, NULL);

    if( !window )
    {
        glfwTerminate();
        exit( -1 );
    }

  const std::string inputGroup = input_name;
  const std::string outputGroup = "LOCAL_CONVERTED_RGB";

  sepia::Reader input( inputGroup );
  sepia::Writer output( outputGroup,
                        input.getGroupHeader()->count,
                        input.getHeader( 0 )->width,
                        input.getHeader( 0 )->height,
                        input.getHeader( 0 )->bpp );

  std::vector< ProcessThread* > threads;
  boost::barrier bar( input.getGroupHeader()->count );

  std::cout << "count: " << input.getGroupHeader()->count << std::endl;

  if( input.getGroupHeader()->count == 1 )
  {
      Settings::show_mode = Settings::show::LEFT; // TODO: handle single window properly.
  }

  for( unsigned int i = 0; i < input.getGroupHeader()->count; i++ )
  {
      threads.push_back( new ProcessThread( &input, &output, &bar, i ) );
  }
  for( unsigned int i = 0; i < threads.size(); i++ )
  {
      threads[i]->start();
  }

  sepia::Reader processed( outputGroup );

  glfwMakeContextCurrent(window);

  glEnable(GL_DEPTH_TEST);



  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);

  glRasterPos2i(0,0);
  //glPixelZoom( 1.0, -1.0 );
  //glPixelZoom( 0.7, -0.7 );
  //glPixelZoom( 0.5, -0.5 );
  //glRasterPos2i(0, window_height/2 );

  glClearColor(0.0, 0.0, 0.0, 1.0);

  glfwSetKeyCallback( window, key_callback );
  glfwSetWindowFocusCallback( window, focus_callback );

  float scale = 1.0;

  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(window))
  {
      /* Render here */
      processed.update();
      size_t height = processed.getHeader( 0 )->height;
      size_t width  = processed.getHeader( 0 )->width;

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      //glDrawPixels(window_width,window_height,GL_RGB,GL_FLOAT,pixels);

      glfwGetFramebufferSize(window, &window_width, &window_height);

      int window_count = 1;
      if( Settings::show_mode == Settings::show::BOTH )
      {
          window_count = 2;
      }

      scale = window_width/(float) ( width * window_count);

      if( Settings::force_noscale )
      {
          scale = 1.0;
          if( window_width != width * window_count )
          {
              window_width = width * window_count;
          }

          if( window_height != height )
          {
              window_height = height;
          }
          glfwSetWindowSize( window, window_width, window_height );
      }


      if( window_height != scale*height )
      {
        window_height = scale*height;
        glfwSetWindowSize( window, window_width, window_height );
      }

      glViewport(0, 0, window_width, window_height );
      glClear(GL_COLOR_BUFFER_BIT);

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0, window_width, window_height, 0, -1, 1);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      glPixelZoom( scale, -scale );
      glRasterPos2i( 0, 0 );

      int color_mode = GL_BGR;
      if( Settings::demosaicing_method == cv::COLOR_BayerBG2GRAY )
      {
          color_mode = GL_LUMINANCE;
      }


      if( Settings::show_mode == Settings::show::BOTH
       || Settings::show_mode == Settings::show::LEFT )
      {
          glDrawPixels( width, height, color_mode, GL_UNSIGNED_BYTE, processed.getAddress( 0 ) );
      }
      if( Settings::show_mode == Settings::show::BOTH )
      {
          glRasterPos2i( width*scale, 0 );
      }
      if( Settings::show_mode == Settings::show::BOTH
       || Settings::show_mode == Settings::show::RIGHT )
      {
          glDrawPixels( width, height, color_mode, GL_UNSIGNED_BYTE, processed.getAddress( 1 ) );
      }

      if( Settings::show_mode == Settings::show::STEREO )
      {
          cv::Mat left_frame( height,
                              width,
                              CV_8UC3,
                              processed.getAddress( 0 ) );

          cv::Mat right_frame( height,
                               width,
                               CV_8UC3,
                               processed.getAddress( 1 ) );

          cv::Mat output_frame( 1024,
                               1280,
                               CV_8UC3 );
          anaglyphic_stereo( left_frame, right_frame, output_frame );
          glDrawPixels( width, height, color_mode, GL_UNSIGNED_BYTE, output_frame.data );
      }

      /* Swap front and back buffers */
      glfwSwapBuffers(window);

      /* Poll for and process events */
      glfwPollEvents();
  }

  glfwTerminate();

  for( unsigned int i = 0; i < threads.size(); i++ )
  {
      threads[i]->stop();
  }

  for( unsigned int i = 0; i < threads.size(); i++ )
  {
      threads[i]->join();
  }
}

