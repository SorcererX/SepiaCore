#ifndef LEPTONCAPTURE_H
#define LEPTONCAPTURE_H
#include <sensorinterface.h>
#include <thread>
#include <atomic>
#include <linux/spi/spidev.h>

namespace sepia
{
    class Writer;
    namespace util
    {
        class ThreadBarrier;
    }
}

class LeptonCapture : public SensorInterface
{
public:
    LeptonCapture( const std::string& a_device );
    ~LeptonCapture();
    void stop();
    void join();
    void start();
    void addSensor( const std::string& a_sensor );
    bool isTerminated();

protected:
    void own_thread();

private:
    unsigned char m_bits;
    unsigned int m_speed;
    struct spi_ioc_transfer m_buffer;
    std::thread* m_thread;
    std::atomic_bool m_terminate;
    sepia::Writer* m_writer;
    int m_fd;

};

#endif // LEPTONCAPTURE_H
