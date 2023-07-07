#ifndef PROCESSTHREAD_H
#define PROCESSTHREAD_H
#include <sepia/util/threadobject.h>
#include <sepia/util/threadbarrier.h>
#include <atomic>
#include <boost/thread/barrier.hpp>

namespace sepia
{
    class Reader;
    class Writer;
    namespace util
    {
        class ThreadBarrier;
    }
}

class ProcessThread : public sepia::util::ThreadObject
{
public:
    ProcessThread( sepia::Reader* a_inputGroup, sepia::Writer* a_outputGroup, sepia::util::ThreadBarrier* a_barrier, int a_id );

protected:
    void own_thread();

private:
    sepia::Reader* m_inputGroup;
    sepia::Writer* m_outputGroup;
    sepia::util::ThreadBarrier* m_barrier;
    int m_id;
};

#endif // PROCESSTHREAD_H
