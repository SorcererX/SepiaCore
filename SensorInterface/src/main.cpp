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

#include <xiApi.h>
#include <signal.h>
#include <iostream>
#include <vector>
#include <ximeacapture.h>
#include <v4l2capture.h>
#include <sensorinterface.h>
#include <sepia/comm/globalreceiver.h>
#include <sepia/comm/observer.h>
#include <sepia/comm/scom.h>
#include <sepia/util/progargs.h>

using sepia::util::ProgArgs;

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
    std::string output_name = "XI_IMG";
    int cameras = 0;
    std::string interface = "XIMEA";


    ProgArgs::init( argc, argv );

    ProgArgs::addOptionDefaults( "interface", &interface, "Interface to use, supported: \"V4L2\", \"XIMEA\"" );
    ProgArgs::addOptionDefaults( "cameras", &cameras, "cameras" );
    ProgArgs::addOptionDefaults( "output_name", &output_name, "Output Group Name" );

    if( ProgArgs::contains( "output_name" ) )
    {
        std::cout << "Using: " << output_name << std::endl;
    }
    sepia::comm::init( "SensorInterface" );
    sepia::comm::GlobalReceiver receiver;
    receiver.start();

    signal(SIGINT, catch_int);
    signal(SIGPIPE, catch_broken_pipe ); // SIG_IGN

    SensorInterface* capture = NULL;

    if( cameras < 1 )
    {
        ProgArgs::printHelp();
        return 1;
    }

    if( interface == "V4L2" )
    {
        capture = new V4L2Capture( cameras );
    }
    else if( interface == "XIMEA" )
    {
        capture = new XimeaCapture( cameras );
    }
    else
    {
        ProgArgs::printHelp();
        return 1;
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

    if( capture )
    {
        capture->join();
    }

    std::cout << "NORMAL_EXIT" << std::endl;
    delete capture;
    return 0;
}
