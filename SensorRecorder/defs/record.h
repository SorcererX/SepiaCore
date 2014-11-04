#ifndef RECORD_H
#define RECORD_H
#include <sepia/util/threadobject.h>

namespace sepia
{
class Reader;
}

class Record : public sepia::util::ThreadObject
{
public:
    Record( const std::string groupname, const std::string filename );
    void start();

protected:
    void own_thread();

private:
    sepia::Reader* m_shared;
    std::string m_fileList;
};

#endif // RECORD_H
