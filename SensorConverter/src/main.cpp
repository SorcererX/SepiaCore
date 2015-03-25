#include <iostream>
#include <sepia/writer.h>
#include <sepia/reader.h>
#include <rectification.h>
#include <processthread.h>
#include <opencv2/opencv.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

using namespace std;

int main( int argc, char** argv )
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

    std::cout << "threads: " << cv::getNumThreads() << std::endl;
    sepia::Reader input( "XI_IMG" );
    sepia::Writer output( "RGB_RECTIFIED", input.getGroupHeader()->count, 1280, 1024, 24 );

    Rectification rec( "intrinsics.yml", "extrinsics.yml" );
    rec.initialize( 1280, 1024 );

    std::vector< ProcessThread* > threads;
    boost::barrier bar( input.getGroupHeader()->count );

    for( unsigned int i = 0; i < input.getGroupHeader()->count; i++ )
    {
        threads.push_back( new ProcessThread( &input, &output, &rec, &bar, i ) );
    }
    for( unsigned int i = 0; i < threads.size(); i++ )
    {
        threads[i]->start();
    }

    for( unsigned int i = 0; i < threads.size(); i++ )
    {
        threads[i]->join();
    }
    return 0;
}

