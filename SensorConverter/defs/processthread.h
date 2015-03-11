#ifndef PROCESSTHREAD_H
#define PROCESSTHREAD_H
#include <thread>
#include <sepia/util/threadobject.h>
#include <atomic>
#include <boost/thread/barrier.hpp>

namespace sepia
{
class Reader;
class Writer;
}
class Rectification;

class ProcessThread : public sepia::util::ThreadObject
{
public:
    ProcessThread( sepia::Reader* input, sepia::Writer* output, Rectification* rectifier, boost::barrier *barrier, int id );
    void start();

protected:
    void own_thread();

private:
    sepia::Reader* m_input;
    sepia::Writer* m_output;
    Rectification* m_rectifier;
    boost::barrier* m_barrier;
    int m_id;
};

#endif // PROCESSTHREAD_H
