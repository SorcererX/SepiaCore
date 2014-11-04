#ifndef V4L2CAPTURE_H
#define V4L2CAPTURE_H
#include "messages.pb.h"
#include <sepia/comm/observer.h>
#include <vector>
#include <atomic>
#include <sensorinterface.h>
#include <boost/thread/mutex.hpp>

class ImageWriter;
class V4L2Camera;

namespace boost {
    class thread;
    class barrier;
}

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
        std::vector< boost::thread* > m_threads;
        std::atomic_bool m_terminate;
        boost::barrier* m_barrier;
        ImageWriter* m_writer;

};
#endif // V4L2CAPTURE_H
