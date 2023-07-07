#ifndef V4L2CAPTURE_H
#define V4L2CAPTURE_H
#include "messages.pb.h"
#include <sepia/comm/observer.h>
#include <vector>
#include <atomic>
#include <thread>
#include <sensorinterface.h>

namespace sepia
{
    class Writer;

    namespace util
    {
        class ThreadBarrier;
    }
}
class V4L2Camera;

class V4L2Capture
      : public SensorInterface
      , public sepia::comm::Observer< cuttlefish_msgs::V4L2Control >
{
    public:
        V4L2Capture( const int devices );
        ~V4L2Capture();
        void stop();
        void join();
        void start();
        void addCamera( const std::string& a_camera );
        bool isTerminated();

    protected:
        void receive( const cuttlefish_msgs::V4L2Control* msg );
        void acquisition_thread( const int camera_no );

    private:
        std::vector< V4L2Camera* > m_cameras;
        std::vector< bool > m_acquisition;
        std::vector< std::thread* > m_threads;
        std::atomic_bool m_terminate;
        sepia::util::ThreadBarrier* m_barrier;
        sepia::Writer* m_writer;

};
#endif // V4L2CAPTURE_H
