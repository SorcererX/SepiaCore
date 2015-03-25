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

#include <iostream>
#include <string>
#include <vector>
#include <sepia/comm/messagehandler.h>
#include <ximeacontroller.h>
#include <unistd.h>
#include <sepia/comm/messagesender.h>
#include <boost/algorithm/string.hpp>

using namespace std;

void testControl()
{
   XimeaController xc;
   xc.addCameraId( 0 );
   xc.addCameraId( 1 );
   xc.open();
   xc.setParameter( "recent_frame", 0 );
   xc.setParameter( "buffers_queue_size", 10 );
   xc.setParameter( "acq_transport_buffer_size", 128 * 1024 );
   xc.setParameter( "imgdataformat", 5 );
   xc.setParameter( "width", 1280 );
   xc.setParameter( "height", 1024 );
   xc.setParameter( "exposure", 30000 );
   xc.setParameter( "gain", 0.0f );
   xc.setParameter( "gammaY", 1.0f );
   xc.setParameter( "gammaC", 0.0f );
   xc.setParameter( "sharpness", 0.0f );
   xc.setParameter( "exp_priority", 1.0f );
   xc.setParameter( "bpc", 1 );
   xc.setParameter( "cms", 1 );
   //xc.setParameter( "limit_bandwidth", 1400 );
   xc.start();
}

void stdinControl()
{
    XimeaController xc;
    std::string command;
    std::vector< std::string > tokens;
    while( getline( std::cin, command ) )
    {
        boost::split( tokens, command, boost::is_any_of(" ") );

        if( tokens.size() == 1 )
        {
            if( tokens.at( 0 ).compare( "open" ) == 0 )
            {
                xc.open();
            }
            else if( tokens.at( 0 ).compare( "close" ) == 0 )
            {
                xc.close();
            }
            else if( tokens.at( 0 ).compare( "start" ) == 0 )
            {
                xc.start();
            }
            else if( tokens.at( 0 ).compare( "stop" ) == 0 )
            {
                xc.stop();
            }
            else if( tokens.at( 0 ).compare( "terminate") == 0 )
            {
                xc.terminate();
            }
            else if( tokens.at( 0 ).compare( "trigger_start" ) == 0 )
            {
                xc.startTrigger();
            }
            else if( tokens.at( 0 ).compare( "trigger_stop" ) == 0 )
            {
                xc.stopTrigger();
            }
            else if( tokens.at( 0 ).compare( "quit" ) == 0 )
            {
                std::cout << "QUIT." << std::endl;
                return ;
            }
        }
        else if( tokens.size() == 2 )
        {
            if( tokens.at( 0 ).compare( "add" ) == 0 )
            {
                int value = std::stoi( tokens.at( 1 ) );
                xc.addCameraId( value );
            }
            else if( tokens.at( 0 ).compare( "remove" ) == 0 )
            {
                int value = std::stoi( tokens.at( 1 ) );
                xc.removeCameraId( value );
            }
        }
        else if( tokens.size() == 3 )
        {
            if( tokens.at( 0 ).compare( "set_int" ) == 0 )
            {
                int value = std::stoi( tokens.at( 2 ) );
                xc.setParameter( tokens.at( 1 ).c_str(), value );
            }
            else if( tokens.at( 0 ).compare( "set_float" ) == 0 )
            {
                float value = std::stof( tokens.at( 2 ) );
                xc.setParameter( tokens.at( 1 ).c_str(), value );
            }
            else if( tokens.at( 0 ).compare( "get_int" ) == 0 )
            {
                std::cerr << "NOT IMPLEMENTED" << std::endl;
            }
            else if( tokens.at( 0 ).compare( "get_float" ) == 0 )
            {
                std::cerr << "NOT IMPLEMENTED" << std::endl;
            }
        }


    }
}




void shut()
{
    XimeaController xc;
    xc.addCameraId( 0 );
    xc.addCameraId( 1 );
    xc.open();
    xc.stop();
    xc.close();
    xc.terminate();

}


int main( int argc, char** argv )
{
   sepia::comm::MessageSender::initClient();
   /*
   QApplication app(argc, argv);
   app.setOrganizationName("Progtec");
   app.setApplicationName("Cuttlefish");
   //
   //testControl();
   //shut();
   SettingsPipeline window;
   window.show();
   return app.exec();
   */
   stdinControl();
}

