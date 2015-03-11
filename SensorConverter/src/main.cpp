#include <iostream>
#include <sepia/writer.h>
#include <sepia/reader.h>
#include <rectification.h>
#include <processthread.h>
#include <opencv2/opencv.hpp>

using namespace std;

int main()
{
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

