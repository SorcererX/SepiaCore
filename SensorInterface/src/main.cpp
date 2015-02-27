//#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <xiApi.h>
#include <signal.h>
#include <iostream>
#include <vector>
#include <boost/program_options.hpp>
#include <ximeacapture.h>
#include <v4l2capture.h>
#include <sensorinterface.h>
#include <sepia/comm/globalreceiver.h>
#include <sepia/comm/observer.h>
#include <sepia/comm/scom.h>

namespace po = boost::program_options;


void catch_int( int ){
    signal(SIGINT, catch_int);
    std::cout << "Ctrl+C interrupted program" << std::endl;
    SensorInterface::terminateAll();
    SensorInterface::joinAll();
}

void catch_broken_pipe( int ) {
    signal(SIGPIPE, catch_broken_pipe );
    std::cout << "GOT_SIGNAL: Broken Pipe" << std::endl;
    std::cout << "IGNORING..." << std::endl;
}

int main( int argc, char *argv[] )
{
    po::options_description desc;

    std::string output_name;
    int cameras;

    desc.add_options()
            ( "v4l2", "Use V4L2" )
            ( "ximea", "Use XIMEA" )
            ( "cameras", po::value<int>(&cameras)->default_value(0), "cameras" )
            ( "output_name",po::value<std::string>(&output_name)->default_value("XI_IMG"), "Output Group Name" );

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

    if( vm.count( "output_name" ) )
    {
        std::cout << "Using: " << output_name << std::endl;
    }
    sepia::comm::init( "SensorInterface" );
    sepia::comm::GlobalReceiver receiver;
    receiver.start();

    signal(SIGINT, catch_int);
    signal(SIGPIPE, catch_broken_pipe ); // SIG_IGN

    SensorInterface* capture = NULL;

    int devices = 0;

    devices += vm.count( "v4l2" );
    devices += vm.count( "ximea" );

    if( devices != 1 || cameras < 1 )
    {
        std::cout << desc << std::endl;
        return 1;
    }

    if( vm.count( "v4l2" ) )
    {
        capture = new V4L2Capture( cameras );
    }
    else if( vm.count( "ximea" ) )
    {
        capture = new XimeaCapture( cameras );
    }

    if( capture )
    {
        capture->start();
    }

    while( !SensorInterface::isAllTerminated() )
    {
        bool handled = sepia::comm::ObserverBase::threadReceiver();

        if( !handled )
        {
            std::cerr << "SUBSCRIPTION_ERROR:" << std::endl;
        }
    }
    SensorInterface::joinAll();
    delete capture;
    return 0;
}
