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

