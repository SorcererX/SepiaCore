#ifndef PROCESSTHREAD_H
#define PROCESSTHREAD_H
#include <sepia/util/threadobject.h>
#include <atomic>
#include <boost/thread/barrier.hpp>

namespace sepia
{
    class Reader;
    class Writer;
}

class ProcessThread : public sepia::util::ThreadObject
{
public:
    ProcessThread( sepia::Reader* inputGroup, sepia::Writer* outputGroup, boost::barrier *barrier, int id );
    void start();

protected:
    void own_thread();

private:
    sepia::Reader* m_inputGroup;
    sepia::Writer* m_outputGroup;
    boost::barrier* m_barrier;
    int m_id;
};

#endif // PROCESSTHREAD_H
