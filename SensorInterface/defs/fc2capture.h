#ifndef FC2CAPTURE_H
#define FC2CAPTURE_H
#include <sepia/comm/observer.h>
#include "messages.pb.h"
#include <vector>
#include <atomic>
#include <sensorinterface.h>
#include <boost/thread/mutex.hpp>
#include "messages.pb.h"

namespace sepia
{
    class Writer;
}

namespace boost {
    class barrier;
}
#include <xiApi.h>


class FC2Capture
      : public SensorInterface
      , public sepia::comm::Observer< cuttlefish_msgs::FC2SetExposure >
{
    public:
        FC2Capture( const std::string& a_outputName, const int devices );
        ~FC2Capture();
        void stop();
        void join();
        void start();
        bool isTerminated();

    protected:
        void receive( const cuttlefish_msgs::FC2SetExposure* a_msg );
        void acquisition_thread( const int a_cameraNo );

    private:
        std::vector< HANDLE > m_handles;
        //std::vector< bool > m_acquisition;
        std::vector< std::thread* > m_threads;
        std::atomic_bool m_terminate;
        boost::barrier* m_barrier;
        boost::barrier* m_triggerBarrier;
        int m_triggerWaitTime;
        sepia::Writer* m_writer;

};

#endif // FC2CAPTURE_H
