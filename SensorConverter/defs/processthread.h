#ifndef PROCESSTHREAD_H
#define PROCESSTHREAD_H
#include <thread>
#include <sepia/util/threadobject.h>
#include <atomic>

namespace sepia
{
    class Reader;
    class Writer;
    namespace util
    {
        class ThreadBarrier;
    }
}
class Rectification;

class ProcessThread : public sepia::util::ThreadObject
{
public:
    ProcessThread( sepia::Reader* a_input, sepia::Writer* a_output, sepia::util::ThreadBarrier* a_barrier, int a_id );
    void setRectification( Rectification* a_rectifier );

protected:
    void own_thread() override;

private:
    sepia::Reader* m_input;
    sepia::Writer* m_output;
    Rectification* m_rectifier;
    sepia::util::ThreadBarrier* m_barrier;
    int m_id;
};

#endif // PROCESSTHREAD_H
