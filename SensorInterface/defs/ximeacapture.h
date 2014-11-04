#ifndef XIMEACAPTURE_H
#define XIMEACAPTURE_H
#include <sepia/comm/observer.h>
#include <vector>
#include <atomic>
#include <sensorinterface.h>
#include <xiApi.h>
#include <boost/thread/mutex.hpp>
#include "messages.pb.h"

namespace sepia
{
    class Writer;
}

namespace boost {
    class thread;
    class barrier;
}
#include <xiApi.h>


class XimeaCapture
      : public SensorInterface,
        public sepia::comm::Observer< cuttlefish_msgs::XimeaControl >,
        public sepia::comm::Observer< cuttlefish_msgs::XimeaSet >,
        public sepia::comm::Observer< cuttlefish_msgs::XimeaGet >
{
    public:
        XimeaCapture( const int devices );
        ~ XimeaCapture();
        void stop();
        void join();
        void start();
        bool isTerminated();

    protected:
        void receive( const cuttlefish_msgs::XimeaControl *msg );
        void receive( const cuttlefish_msgs::XimeaSet *msg );
        void receive( const cuttlefish_msgs::XimeaGet *msg );
        void acquisition_thread( const int camera_no );

    private:
        std::vector< HANDLE > m_handles;
        std::vector< bool > m_acquisition;
        std::vector< bool > m_trigger;
        std::vector< boost::thread* > m_threads;
        std::atomic_bool m_terminate;
        boost::barrier* m_barrier;
        boost::barrier* m_triggerBarrier;
        int m_triggerWaitTime;
        sepia::Writer* m_writer;

};

#endif // XIMEACAPTURE_H
