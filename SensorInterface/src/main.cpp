//#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <xiApi.h>
#include <signal.h>
#include <iostream>
#include <vector>
#include <ximeacapture.h>
#include <sensorinterface.h>
#include <sepia/comm/globalreceiver.h>
#include <sepia/comm/observer.h>
#include <sepia/comm/scom.h>

bool m_terminate = false;


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
   sepia::comm::init( "SensorInterface" );
   sepia::comm::GlobalReceiver receiver;
   receiver.start();

   signal(SIGINT, catch_int);
   signal(SIGPIPE, catch_broken_pipe ); // SIG_IGN
   XimeaCapture xiCapture( 2 );
   xiCapture.start();
   while( !xiCapture.isTerminated() )
   {
      bool handled = sepia::comm::ObserverBase::threadReceiver();

      if( !handled )
      {
         std::cerr << "SUBSCRIPTION_ERROR:" << std::endl;
      }
   }

   xiCapture.join();
   return 0;
}
