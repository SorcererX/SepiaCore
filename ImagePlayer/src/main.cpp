#include <iostream>
#include <imageplayer.h>

int main()
{
    ImagePlayer player( "IMAGEPLAYER" );
    player.setInputFile( "input3.list");
    player.enableLoop( true );
    player.start();
    player.join();
    return 0;
}

