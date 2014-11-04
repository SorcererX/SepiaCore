#ifndef PLAYBACK_H
#define PLAYBACK_H
#include <sepia/util/threadobject.h>
#include <sepia/writer.h>

class Playback : public sepia::util::ThreadObject
{
public:
    Playback( const std::string& groupname, const std::string& file_base );
    void useRealFps( bool enable );
    void setFps( double fps );
    void enableLoop( bool enable );
    void start();

protected:
    void own_thread();
    bool readImages( std::ifstream& input, sepia::Writer& output );

private:
    std::string m_groupName;
    std::string m_recFileList;
    double m_fps;
    bool m_useRealFps;
    bool m_loop;
};


#endif // PLAYBACK_H
