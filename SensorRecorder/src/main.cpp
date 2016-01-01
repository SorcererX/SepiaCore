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
#include <sepia/util/progargs.h>
#include <record.h>
#include <playback.h>
#include <unistd.h>

using sepia::util::ProgArgs;

int main( int argc, char** argv )
{

    ProgArgs::init( argc, argv );
    std::string input_name = "XI_IMG";
    std::string filename = "output.list";
    double fps = 15.0;
    int timeout = 0;

    ProgArgs::addOptionDefaults( "record", "Record" );
    ProgArgs::addOptionDefaults( "playback", "Playback" );
    ProgArgs::addOptionDefaults( "fps", &fps, "Frames per second" );
    ProgArgs::addOptionDefaults( "timeout", &timeout, "Seconds to record" );
    ProgArgs::addOptionDefaults( "real_fps", "Use actual recording fps" );
    ProgArgs::addOptionDefaults( "filename", &filename, "File Name" );
    ProgArgs::addOptionDefaults( "loop", "Loop indefinitely" );
    ProgArgs::addOptionDefaults( "input_name", &input_name, "Input Group Name" );

    if(  (  ProgArgs::contains( "record") && ProgArgs::contains( "playback" ) )
      || ( !ProgArgs::contains( "record" ) && !ProgArgs::contains( "playback" ) ) )
    {
        ProgArgs::printHelp();
        return 1;
    }

    if( ProgArgs::contains( "record" ) )
    {
        Record rec( input_name, filename );
        rec.start();
        if( timeout != 0 )
        {
            sleep( timeout );
            rec.stop();
        }
        rec.join();
    }

    if( ProgArgs::contains( "playback" ) )
    {
        Playback play( input_name, filename );
        if( ProgArgs::contains( "loop") )
        {
            play.enableLoop( true );
        }
        if( ProgArgs::contains( "real_fps" ) )
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

