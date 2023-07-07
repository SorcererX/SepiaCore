#ifndef ARAVISCAPTURE_H
#define ARAVISCAPTURE_H
#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include "sensorinterface.h"

namespace sepia
{
    class Writer;

    namespace util
    {
        class ThreadBarrier;
    }
}

class AravisCapture : public SensorInterface
{
public:
    AravisCapture( std::size_t a_cameras );
    ~AravisCapture();
    void stop();
    void join();
    void start();
    void addCamera( const std::string& a_camera );
    bool isTerminated();

protected:
    void acquisition_thread( std::size_t a_id, const std::string& a_deviceString );

private:
    std::size_t m_cameras{ 0 };
    std::vector< std::unique_ptr< std::thread > > m_threads;
    sepia::util::ThreadBarrier* m_barrier;
    sepia::Writer* m_writer;
    std::atomic_bool m_terminate{ false };
};
#endif // ARAVISCAPTURE_H
