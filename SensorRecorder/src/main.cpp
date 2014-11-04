#include <iostream>
#include <boost/program_options.hpp>
#include <record.h>
#include <playback.h>

namespace po = boost::program_options;

int main( int argc, char** argv )
{
    po::options_description desc;

    std::string group_name;
    std::string filename;
    double fps;
    int timeout;

    desc.add_options()
            ("record", "Record" )
            ("playback", "Playback" )
            ( "fps", po::value<double>(&fps)->default_value(15.0), "Frames per second" )
            ( "timeout", po::value<int>(&timeout)->default_value(0), "Seconds to record" )
            ( "real_fps", "Use actual recording fps" )
            ( "filename", po::value<std::string>(&filename)->default_value( "output.list" ), "File Name" )
            ( "loop", "Loop indefinitely" )
            ( "group_name",po::value<std::string>(&group_name)->default_value("XI_IMG"), "Group Name" );

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

    if(  (  vm.count( "record") &&  vm.count( "playback" ) )
      || ( !vm.count( "record") && !vm.count( "playback" ) ) )
    {
        std::cout << desc << std::endl;
        return 1;
    }

    if( vm.count( "record" ) )
    {
        Record rec( group_name, filename );
        rec.start();
        if( timeout != 0 )
        {
            sleep( timeout );
            rec.stop();
        }
        rec.join();
    }

    if( vm.count( "playback" ) )
    {
        Playback play( group_name, filename );
        if( vm.count( "loop") )
        {
            play.enableLoop( true );
        }
        if( vm.count( "real_fps" ) )
        {
            play.useRealFps( true );
        }
        else
        {
            play.useRealFps( false );
            play.setFps( fps );
        }
        play.start();
        play.join();
    }


}

