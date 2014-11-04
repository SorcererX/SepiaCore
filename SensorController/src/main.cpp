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

